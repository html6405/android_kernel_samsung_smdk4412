/* Pure, length-checked eligibility predicate shared with packet tests.
 * Caller provides u8/bool/memcmp. Only direct, empty, zero-lifetime RAs qualify.
 */
#ifndef _DHD_RA_PACKET_H
#define _DHD_RA_PACKET_H
struct dhd_ra_observation {
	u8 packet[78];
	unsigned int seen;
	bool tainted;
};
static bool dhd_ra_empty_packet(const u8 *p, unsigned int len)
{
	return len == 78 && p[12] == 0x86 && p[13] == 0xdd &&
		(p[14] >> 4) == 6 && p[18] == 0 && p[19] == 24 &&
		p[20] == 58 && p[21] == 255 && p[54] == 134 && p[55] == 0 &&
		p[60] == 0 && p[61] == 0 && p[70] == 1 && p[71] == 1 &&
		!memcmp(p + 72, p + 6, 6);
}

/* Caller serializes observations. True requests a worker on a state change. */
static bool dhd_ra_observe(struct dhd_ra_observation *o, const u8 *p,
			  bool eligible)
{
	if (!eligible || (o->seen && memcmp(o->packet, p, 78))) {
		if (!o->tainted) {
			o->tainted = true;
			return true;
		}
	} else if (!o->tainted && o->seen < 3) {
		memcpy(o->packet, p, 78);
		return ++o->seen == 3;
	}
	return false;
}
#endif
