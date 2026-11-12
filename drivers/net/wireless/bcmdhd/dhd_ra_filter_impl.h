/* Included once by dhd_linux.c, after its private structure/filter helpers.
 * Experimental and disabled by default. No software RX packet is dropped.
 * Only a repeatedly observed, byte-identical empty withdrawal RA is offloaded.
 * Other/changed RAs invalidate the experiment for the current screen-off epoch.
 */
#ifdef DHD_RA_FILTER_TRIAL
#include <net/ipv6.h>
#include "dhd_ra_packet.h"
static bool dhd_ra_filter;
module_param_named(ra_filter, dhd_ra_filter, bool, 0644);
MODULE_PARM_DESC(ra_filter, "Experimental P4NOTE duplicate empty-RA offload; default off");

#define DHD_RA_BLOCK_SECONDS 120
#define DHD_RA_OPEN_SECONDS 30

static s64 dhd_ra_now(void)
{
	struct timespec ts;
	get_monotonic_boottime(&ts);
	return timespec_to_ns(&ts);
}

static int dhd_ra_iovar(struct dhd_ra_state *s, const char *name,
			const void *data, unsigned int len)
{
	u8 buf[256];
	unsigned int n = strlen(name) + 1;
	if (n + len > sizeof(buf))
		return -EINVAL;
	memcpy(buf, name, n);
	memcpy(buf + n, data, len);
	return dhd_wl_ioctl_cmd(&s->info->pub, WLC_SET_VAR, buf,
				n + len, TRUE, 0);
}

static int dhd_ra_int(struct dhd_ra_state *s, const char *name, u32 value)
{
	value = htod32(value);
	return dhd_ra_iovar(s, name, &value, sizeof(value));
}

static int dhd_ra_enable(struct dhd_ra_state *s, u32 id, bool enabled)
{
	wl_pkt_filter_enable_t e;
	e.id = htod32(id);
	e.enable = htod32(enabled);
	return dhd_ra_iovar(s, "pkt_filter_enable", &e, sizeof(e));
}

/* Refuse unfamiliar policies and IDs owned by another firmware client.
 * The equivalent common-L2 blacklist is only tested for rules 100/103.
 */
static int dhd_ra_policy(struct dhd_ra_state *s, bool enabled)
{
	u8 buf[512];
	u32 selector = htod32(enabled), count, id, size;
	unsigned int pos, i, found = 0;
	wl_pkt_filter_t *f;
	int ret;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "pkt_filter_list", sizeof("pkt_filter_list"));
	memcpy(buf + sizeof("pkt_filter_list"), &selector, sizeof(selector));
	ret = dhd_wl_ioctl_cmd(&s->info->pub, WLC_GET_VAR, buf,
			     sizeof(buf), FALSE, 0);
	if (ret < 0)
		return ret;
	memcpy(&count, buf, sizeof(count));
	count = dtoh32(count);
	if (count > 16 || (enabled && count != 2))
		return -EINVAL;
	pos = sizeof(count);
	for (i = 0; i < count; i++) {
		if (pos + 20 > sizeof(buf))
			return -EINVAL;
		f = (wl_pkt_filter_t *)(buf + pos);
		id = dtoh32(f->id);
		size = dtoh32(f->u.pattern.size_bytes);
		if (dtoh32(f->type) != 0 || size > 128 ||
		    pos + 20 + 2 * size > sizeof(buf) ||
		    (id >= 200 && id <= 202))
			return -EINVAL;
		if (enabled) {
			if (dtoh32(f->negate_match) || dtoh32(f->u.pattern.offset))
				return -EINVAL;
			if (id == 100 && size == 1 &&
			    !memcmp(f->u.pattern.mask_and_pattern, "\x01\x00", 2))
				found |= 1;
			else if (id == 103 && size == 2 &&
				 !memcmp(f->u.pattern.mask_and_pattern, "\xff\xff\x33\x33", 4))
				found |= 2;
			else
				return -EINVAL;
		}
		pos += ALIGN(20 + 2 * size, 4);
	}
	return enabled && found != 3 ? -EINVAL : 0;
}

static int dhd_ra_add(struct dhd_ra_state *s, u32 id,
		       const u8 *mask, const u8 *pattern, unsigned int len)
{
	u8 buf[20 + 2 * 78];
	wl_pkt_filter_t *f = (wl_pkt_filter_t *)buf;
	int ret;
	memset(buf, 0, sizeof(buf));
	f->id = htod32(id);
	f->u.pattern.size_bytes = htod32(len);
	memcpy(f->u.pattern.mask_and_pattern, mask, len);
	memcpy(f->u.pattern.mask_and_pattern + len, pattern, len);
	ret = dhd_ra_iovar(s, "pkt_filter_add", buf, 20 + 2 * len);
	if (ret >= 0)
		s->installed |= 1U << (id - 200);
	return ret;
}

/* Change back to normal forwarding before deleting trial rules: fail open.
 * Keep failed rule IDs for a retry at the next normal filter transition.
 */
static int dhd_ra_restore(struct dhd_ra_state *s)
{
	unsigned int i;
	int error = 0, ret;
	bool was_active = s->active;
	if (!s->installed)
		return 0;
	if (!s->info->pub.up) {
		/* Firmware has been/will be reset by the ordinary down/up path. */
		s->installed = 0;
		s->active = false;
		return 0;
	}
	ret = dhd_ra_int(s, "pkt_filter_mode", 1);
	if (ret < 0)
		error = ret;
	ret = dhd_ra_enable(s, 100, true);
	if (ret < 0)
		error = ret;
	ret = dhd_ra_enable(s, 103, true);
	if (ret < 0)
		error = ret;
	for (i = 0; i < 3; i++) {
		if (!(s->installed & (1U << i)))
			continue;
		ret = dhd_ra_int(s, "pkt_filter_delete", 200 + i);
		if (ret >= 0)
			s->installed &= ~(1U << i);
		else
			error = ret;
	}
	s->active = false;
	if (error)
		DHD_ERROR(("RA offload: restore failed %d; trial stopped\n", error));
	else if (was_active)
		DHD_ERROR(("RA offload: normal filters restored\n"));
	return error;
}

static int dhd_ra_apply(struct dhd_ra_state *s, const u8 *packet)
{
	u8 mask[78], buf[32];
	u32 mode;
	int ret;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "pkt_filter_mode", sizeof("pkt_filter_mode"));
	ret = dhd_wl_ioctl_cmd(&s->info->pub, WLC_GET_VAR, buf, sizeof(buf), FALSE, 0);
	memcpy(&mode, buf, sizeof(mode));
	if (ret < 0 || dtoh32(mode) != 1 ||
	    dhd_ra_policy(s, true) || dhd_ra_policy(s, false))
		return -EINVAL;
	memset(mask, 0xff, sizeof(mask));
	ret = dhd_ra_add(s, 200, mask, packet, 78);
	if (ret < 0)
		goto fail;
	ret = dhd_ra_add(s, 201, mask, (const u8 *)"\xff\xff\xff\xff\xff\xff", 6);
	if (ret < 0)
		goto fail;
	ret = dhd_ra_add(s, 202, mask, (const u8 *)"\x01\x00\x5e", 3);
	if (ret < 0)
		goto fail;
	if ((ret = dhd_ra_enable(s, 100, false)) < 0 ||
	    (ret = dhd_ra_enable(s, 103, false)) < 0 ||
	    (ret = dhd_ra_enable(s, 200, true)) < 0 ||
	    (ret = dhd_ra_enable(s, 201, true)) < 0 ||
	    (ret = dhd_ra_enable(s, 202, true)) < 0 ||
	    (ret = dhd_ra_int(s, "pkt_filter_mode", 0)) < 0)
		goto fail;
	s->active = true;
	DHD_ERROR(("RA offload: repeated empty RA filtered; refresh target %ds\n",
		   DHD_RA_BLOCK_SECONDS));
	return 0;
fail:
	dhd_ra_restore(s);
	return ret;
}

static void dhd_ra_kick(struct dhd_ra_state *s)
{
	unsigned long flags;
	spin_lock_irqsave(&s->rx_lock, flags);
	if (ACCESS_ONCE(s->wanted)) {
		__pm_stay_awake(&s->wake);
		schedule_work(&s->work);
	}
	spin_unlock_irqrestore(&s->rx_lock, flags);
}

static enum alarmtimer_restart dhd_ra_alarm(struct alarm *alarm, ktime_t now)
{
	struct dhd_ra_state *s = container_of(alarm, struct dhd_ra_state, alarm);
	if (ACCESS_ONCE(s->wanted))
		dhd_ra_kick(s);
	return ALARMTIMER_NORESTART;
}

static void dhd_ra_work(struct work_struct *work)
{
	struct dhd_ra_state *s = container_of(work, struct dhd_ra_state, work);
	unsigned long flags;
	u8 packet[78];
	bool ready;
	s64 remaining;
	mutex_lock(&s->mutex);
	spin_lock_irqsave(&s->rx_lock, flags);
	ready = s->observed.seen >= 3 && !s->observed.tainted;
	memcpy(packet, s->observed.packet, sizeof(packet));
	spin_unlock_irqrestore(&s->rx_lock, flags);
	if (!dhd_ra_filter || !s->wanted || !ready || s->faulted ||
	    !s->info->pub.up || !s->info->pub.in_suspend ||
	    s->info->pub.dhcp_in_progress || s->info->pub.suspend_disable_flag ||
	    !dhd_support_sta_mode(&s->info->pub) ||
	    (s->info->pub.op_mode & (DHD_FLAG_HOSTAP_MODE | DHD_FLAG_P2P_GC_MODE | DHD_FLAG_P2P_GO_MODE))) {
		alarm_cancel(&s->alarm);
		if (dhd_ra_restore(s))
			s->faulted = true;
		goto out;
	}
	if (s->active) {
		if (dhd_ra_restore(s)) {
			s->faulted = true;
			goto out;
		}
		s->open_until = dhd_ra_now() + (s64)DHD_RA_OPEN_SECONDS * NSEC_PER_SEC;
	}
	remaining = s->open_until - dhd_ra_now();
	if (remaining > 0) {
		alarm_start_relative(&s->alarm, ns_to_ktime(remaining));
		goto out;
	}
	if (dhd_ra_apply(s, packet)) {
		s->faulted = true;
		DHD_ERROR(("RA offload: unsupported policy/firmware; trial stopped\n"));
		goto out;
	}
	alarm_start_relative(&s->alarm, ktime_set(DHD_RA_BLOCK_SECONDS, 0));
out:
	mutex_unlock(&s->mutex);
	spin_lock_irqsave(&s->rx_lock, flags);
	/* A new RX/alarm event may have queued this work again while it ran. */
	if (!work_pending(&s->work))
		__pm_relax(&s->wake);
	spin_unlock_irqrestore(&s->rx_lock, flags);
}

static void dhd_ra_init(struct dhd_info *dhd)
{
	struct dhd_ra_state *s = &dhd->ra;
	s->info = dhd;
	mutex_init(&s->mutex);
	spin_lock_init(&s->rx_lock);
	INIT_WORK(&s->work, dhd_ra_work);
	alarm_init(&s->alarm, ALARM_BOOTTIME, dhd_ra_alarm);
	wakeup_source_init(&s->wake, "wlan_ra_transition");
	s->initialized = true;
}

/* Hold this mutex through the ordinary firmware filter update as well. */
static void dhd_ra_begin(struct dhd_info *dhd, bool value)
{
	struct dhd_ra_state *s = &dhd->ra;
	unsigned long flags;
	if (!s->initialized)
		return;
	mutex_lock(&s->mutex);
	ACCESS_ONCE(s->wanted) = false;
	/* Serialize cancellation with work that can rearm the alarm. */
	alarm_cancel(&s->alarm);
	if (dhd_ra_restore(s))
		s->faulted = true;
	if (!value) {
		spin_lock_irqsave(&s->rx_lock, flags);
		s->observed.seen = 0;
		s->observed.tainted = false;
		spin_unlock_irqrestore(&s->rx_lock, flags);
		if (!s->installed)
			s->faulted = false;
	}
	s->open_until = dhd_ra_now() + (s64)DHD_RA_OPEN_SECONDS * NSEC_PER_SEC;
}

static void dhd_ra_end(struct dhd_info *dhd, bool value)
{
	struct dhd_ra_state *s = &dhd->ra;
	bool wanted;
	if (!s->initialized)
		return;
	wanted = value && dhd_ra_filter && dhd->pub.in_suspend;
	ACCESS_ONCE(s->wanted) = wanted;
	mutex_unlock(&s->mutex);
	if (wanted)
		dhd_ra_kick(s);
}

static void dhd_ra_before(struct dhd_info *dhd, bool value)
{
	dhd_ra_begin(dhd, value);
	dhd_ra_end(dhd, false);
}

static void dhd_ra_rx(struct dhd_info *dhd, int ifidx,
		      const struct sk_buff *skb)
{
	struct dhd_ra_state *s = &dhd->ra;
	u8 prefix[78];
	const u8 *p = skb->data;
	unsigned int len = skb->len;
	unsigned long flags;
	bool kick = false, empty, extension;
	int offset;
	u8 next, type;
	__be16 fragment;
	if (!dhd_ra_filter || !s->initialized || !ACCESS_ONCE(s->wanted) ||
	    ifidx || len < ETH_HLEN)
		return;
	if (skb_is_nonlinear(skb)) {
		if (skb_copy_bits(skb, 0, prefix, min_t(unsigned int, len, sizeof(prefix))))
			return;
		p = prefix;
	}
	/* The small blacklist is not a complete MAC-policy complement. If an
	 * uncommon group address gets through, stop this trial instead of
	 * repeatedly waking for traffic the old whitelist would have blocked. */
	if ((p[0] & 1) && (p[0] != 0x33 || p[1] != 0x33)) {
		spin_lock_irqsave(&s->rx_lock, flags);
		if (ACCESS_ONCE(s->active))
			kick = dhd_ra_observe(&s->observed, NULL, false);
		spin_unlock_irqrestore(&s->rx_lock, flags);
		if (kick)
			dhd_ra_kick(s);
		return;
	}
	if (len < 54 || p[12] != 0x86 || p[13] != 0xdd ||
	    (p[14] >> 4) != 6)
		return;
	/* Parse only to recognize changed RAs; never offload their extended form.
	 * In particular, ordinary MLD reports carry Hop-by-Hop headers too and
	 * must not be mistaken for changed router advertisements. */
	extension = p[20] == 0 || p[20] == 43 || p[20] == 44 ||
		    p[20] == 51 || p[20] == 60;
	if (extension) {
		next = p[20];
		offset = ipv6_skip_exthdr(skb, 54, &next, &fragment);
		if (offset < 0 || (unsigned int)offset >= len)
			return;
		if (!(ntohs(fragment) & ~0x7)) {
			if (next != 58 || skb_copy_bits(skb, offset, &type, 1) || type != 134)
				return;
		}
		/* Non-first fragments cannot be classified here: fail conservatively. */
	} else if (len < 70 || p[20] != 58 || p[54] != 134) {
		return;
	}
	/* Only the observed empty RA: no prefix, route, RDNSS or MTU option.
	 * Byte-identical matching includes checksum, source, flags and destination.
	 * Changed/non-empty RAs are always delivered and disable this epoch's trial.
	 */
	empty = !extension && dhd_ra_empty_packet(p, len);
	spin_lock_irqsave(&s->rx_lock, flags);
	kick = dhd_ra_observe(&s->observed, p, empty);
	spin_unlock_irqrestore(&s->rx_lock, flags);
	if (kick)
		dhd_ra_kick(s);
}

static void dhd_ra_quiesce(struct dhd_info *dhd)
{
	struct dhd_ra_state *s = &dhd->ra;
	if (!s->initialized)
		return;
	dhd_ra_before(dhd, false);
	cancel_work_sync(&s->work);
	__pm_relax(&s->wake);
}

static void dhd_ra_destroy(struct dhd_info *dhd)
{
	if (dhd->ra.initialized) {
		dhd_ra_quiesce(dhd);
		wakeup_source_trash(&dhd->ra.wake);
		dhd->ra.initialized = false;
	}
}
#endif /* DHD_RA_FILTER_TRIAL */
