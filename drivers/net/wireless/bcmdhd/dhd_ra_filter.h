/* Optional P4NOTE firmware offload for repeated, empty IPv6 RAs. */
#ifndef _DHD_RA_FILTER_H
#define _DHD_RA_FILTER_H

#if defined(CONFIG_MACH_P4NOTE) && defined(PKT_FILTER_SUPPORT) && \
    defined(DHD_USE_WAKEUP_SOURCES)
#define DHD_RA_FILTER_TRIAL
#include <linux/alarmtimer.h>
#include "dhd_ra_packet.h"

struct dhd_info;
struct dhd_ra_state {
	struct dhd_info *info;
	struct mutex mutex;
	spinlock_t rx_lock;
	struct alarm alarm;
	struct work_struct work;
	struct wakeup_source wake;
	bool initialized, wanted, active, faulted;
	unsigned int installed;
	s64 open_until;
	struct dhd_ra_observation observed;
};
static void dhd_ra_init(struct dhd_info *dhd);
static void dhd_ra_begin(struct dhd_info *dhd, bool value);
static void dhd_ra_end(struct dhd_info *dhd, bool value);
static void dhd_ra_before(struct dhd_info *dhd, bool value);
static void dhd_ra_rx(struct dhd_info *dhd, int ifidx,
		      const struct sk_buff *skb);
static void dhd_ra_quiesce(struct dhd_info *dhd);
static void dhd_ra_destroy(struct dhd_info *dhd);
#else
#define dhd_ra_init(dhd) do { } while (0)
#define dhd_ra_begin(dhd, value) do { } while (0)
#define dhd_ra_end(dhd, value) do { } while (0)
#define dhd_ra_before(dhd, value) do { } while (0)
#define dhd_ra_rx(dhd, idx, skb) do { } while (0)
#define dhd_ra_quiesce(dhd) do { } while (0)
#define dhd_ra_destroy(dhd) do { } while (0)
#endif
#endif
