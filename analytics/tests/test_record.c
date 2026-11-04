/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <analytics/persistence.h>

int main(void) {
    struct analytics_saved_record records[2] = {0}, decoded;
    enum analytics_slot_state states[2] = {ANALYTICS_SLOT_EMPTY, ANALYTICS_SLOT_EMPTY};
    uint8_t bytes[ANALYTICS_RECORD_SIZE], corrupt[ANALYTICS_RECORD_SIZE];
    uint8_t identity[32] = {0};
    assert(ANALYTICS_RECORD_SIZE == 408);
    assert(analytics_choose_record(records, states, identity) == -ENOENT);
    records[0].generation = 1;
    records[0].stats.presses[0][0] = 0x12345678;
    records[0].stats.presses[41][1] = UINT32_MAX;
    records[0].stats.releases[0] = 7;
    records[0].stats.invalid_positions = 9;
    records[0].stats.saturated = true;
    analytics_encode_record(bytes, &records[0]);
    assert(bytes[56] == 0x78 && bytes[59] == 0x12);
    assert(analytics_decode_record(&decoded, bytes, sizeof(bytes)) == 0);
    assert(decoded.generation == 1 && analytics_stats_equal(&decoded.stats, &records[0].stats));
    for (size_t n = 0; n < sizeof(bytes); ++n) {
        assert(analytics_decode_record(&decoded, bytes, n) == -EBADMSG);
        memcpy(corrupt, bytes, sizeof(bytes));
        corrupt[n] ^= 1;
        assert(analytics_decode_record(&decoded, corrupt, sizeof(corrupt)) == -EBADMSG);
    }
    states[0] = ANALYTICS_SLOT_VALID;
    assert(analytics_choose_record(records, states, identity) == 0);
    states[1] = ANALYTICS_SLOT_BAD; /* interrupted/corrupt newer slot: keep older */
    assert(analytics_choose_record(records, states, identity) == 0);
    states[0] = ANALYTICS_SLOT_BAD;
    assert(analytics_choose_record(records, states, identity) == -EBADMSG);
    states[0] = states[1] = ANALYTICS_SLOT_VALID;
    records[1] = records[0];
    records[1].generation = 2;
    assert(analytics_choose_record(records, states, identity) == 1);
    records[1].layout_hash[0] = 1;
    assert(analytics_choose_record(records, states, identity) == -EXDEV);
    records[1] = records[0];
    records[1].stats.presses[0][0]++;
    assert(analytics_choose_record(records, states, identity) == -EILSEQ);
    records[1] = records[0];
    records[1].generation = UINT32_MAX;
    assert(analytics_choose_record(records, states, identity) == 1);
    struct analytics_poc_snapshot early = {0};
    early.presses[0][0] = 3;
    early.presses[41][1] = 10;
    analytics_poc_merge(&early, &records[0].stats);
    assert(early.presses[0][0] == 0x1234567B);
    assert(early.presses[41][1] == UINT32_MAX && early.saturated);
    assert(early.releases[0] == 7 && early.invalid_positions == 9);
    puts("PASS: binary roundtrip, every-byte corruption/truncation, fallback, layout, merge");
    return 0;
}
