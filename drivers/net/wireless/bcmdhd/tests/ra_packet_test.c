#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;
#include "../dhd_ra_packet.h"

int main(int argc, char **argv)
{
    u8 p[96] = {0};
    struct dhd_ra_observation o = {0};
    unsigned int n;

    if (argc == 2) {
        FILE *f = fopen(argv[1], "rb");
        assert(f);
        n = fread(p, 1, sizeof(p), f);
        assert(!ferror(f));
        fclose(f);
        assert(dhd_ra_empty_packet(p, n));
        memset(p, 0, sizeof(p));
        puts("Captured RA accepted by actual driver predicate");
    }
    p[6] = 0x02; p[11] = 1;
    p[12] = 0x86; p[13] = 0xdd; p[14] = 0x60;
    p[19] = 24; p[20] = 58; p[21] = 255; p[54] = 134;
    p[59] = 0xc0; p[70] = 1; p[71] = 1;
    memcpy(p + 72, p + 6, 6);
    assert(dhd_ra_empty_packet(p, 78));
    assert(!dhd_ra_observe(&o, p, true));
    assert(!dhd_ra_observe(&o, p, true));
    assert(dhd_ra_observe(&o, p, true));
    assert(o.seen == 3 && !o.tainted);
    assert(!dhd_ra_observe(&o, p, true));
    p[59] ^= 0x40;
    assert(dhd_ra_empty_packet(p, 78));
    assert(dhd_ra_observe(&o, p, true));
    assert(o.tainted);
    assert(!dhd_ra_observe(&o, p, true));
    p[59] ^= 0x40;
    assert(!dhd_ra_observe(&o, p, true));
    memset(&o, 0, sizeof(o));
    assert(dhd_ra_observe(&o, NULL, false));
    assert(o.tainted && !dhd_ra_observe(&o, NULL, false));

    /* Exact-sized arrays exercise truncation paths under ASan. */
    for (n = 0; n < 78; n++) {
        u8 short_packet[n ? n : 1];
        memcpy(short_packet, p, n);
        assert(!dhd_ra_empty_packet(short_packet, n));
    }
    assert(!dhd_ra_empty_packet(p, 96));
    p[60] = 7; p[61] = 8;
    assert(!dhd_ra_empty_packet(p, 78));
    p[60] = p[61] = 0;
    p[19] = 56; assert(!dhd_ra_empty_packet(p, 78));
    p[19] = 24;
    p[20] = 0; assert(!dhd_ra_empty_packet(p, 78));
    p[20] = 58;
    p[21] = 64; assert(!dhd_ra_empty_packet(p, 78));
    p[21] = 255;
    p[54] = 135; assert(!dhd_ra_empty_packet(p, 78));
    p[54] = 134;
    p[70] = 25; assert(!dhd_ra_empty_packet(p, 78));
    p[70] = 1;
    p[72] ^= 1; assert(!dhd_ra_empty_packet(p, 78));
    puts("RA eligibility and observation state: PASS");
    return 0;
}
