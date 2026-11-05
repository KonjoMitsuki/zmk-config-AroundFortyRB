/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <analytics/persistence.h>

static uint32_t crc32(const uint8_t *p, size_t size) {
    uint32_t crc = UINT32_MAX;
    while (size--) {
        crc ^= *p++;
        for (unsigned int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
        }
    }
    return ~crc;
}

static void put32(uint8_t *p, uint32_t n) {
    for (unsigned int i = 0; i < 4; ++i) {
        p[i] = (uint8_t)(n >> (i * 8));
    }
}

static void make_v1(uint8_t *bytes, const struct analytics_saved_record *record) {
    put32(bytes, 0x31414B5Au);
    put32(bytes + 4, 1);
    put32(bytes + 8, ANALYTICS_RECORD_SIZE_V1);
    put32(bytes + 12, ANALYTICS_POSITION_COUNT);
    put32(bytes + 16, record->generation);
    memcpy(bytes + 20, record->layout_hash, 32);
    put32(bytes + 52, record->stats.saturated ? 1 : 0);
    size_t offset = 56;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
            put32(bytes + offset, record->stats.presses[p][s]);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
        put32(bytes + offset, record->stats.releases[s]);
    }
    put32(bytes + offset, record->stats.invalid_positions);
    put32(bytes + ANALYTICS_RECORD_SIZE_V1 - 4,
          crc32(bytes, ANALYTICS_RECORD_SIZE_V1 - 4));
}

int main(void) {
    struct analytics_saved_record records[2] = {0}, decoded;
    enum analytics_slot_state states[2] = {ANALYTICS_SLOT_EMPTY, ANALYTICS_SLOT_EMPTY};
    uint8_t bytes[ANALYTICS_RECORD_SIZE], corrupt[ANALYTICS_RECORD_SIZE];
    uint8_t v1[ANALYTICS_RECORD_SIZE_V1];
    uint8_t identity[32] = {0};
    assert(ANALYTICS_RECORD_SIZE_V1 == 408);
    assert(ANALYTICS_RECORD_SIZE == 3632);
    assert(ANALYTICS_BIGRAM_RECORD_SIZE == 3596);

    assert(analytics_choose_record(records, states, identity) == -ENOENT);
    analytics_poc_init(&records[0].stats, "0:1,2:3");
    records[0].generation = 1;
    records[0].stats.presses[0][0] = 0x12345678;
    records[0].stats.presses[41][1] = UINT32_MAX;
    records[0].stats.releases[0] = 7;
    records[0].stats.invalid_positions = 9;
    records[0].stats.duplicate_downs = 2;
    records[0].stats.orphan_releases = 3;
    records[0].stats.timing_discards = 4;
    records[0].stats.pair_config_errors = 5;
    records[0].stats.saturated = true;
    records[0].stats.histogram_saturated = true;
    records[0].stats.hold[3][2] = 11;
    records[0].stats.same_key[4][7] = 12;
    records[0].stats.idle_before_press[5][0] = 13;
    records[0].stats.pair_near[0][ANALYTICS_TIMING_CC][1] = 14;
    records[0].stats.pair_overlap[0][ANALYTICS_TIMING_CP][5] = 15;
    records[0].layout_hash[0] = 0xA5;
    analytics_encode_record(bytes, &records[0]);
    assert(bytes[0] == 0x5A && bytes[1] == 0x4B && bytes[2] == 0x41 && bytes[3] == 0x32);
    assert(bytes[64] == 0 && bytes[65] == 1);
    assert(analytics_decode_record(&decoded, bytes, sizeof(bytes)) == 0);
    assert(decoded.generation == 1 && decoded.schema_version == 2);
    assert(decoded.layout_hash[0] == 0xA5);
    assert(analytics_stats_equal(&decoded.stats, &records[0].stats));

    for (size_t n = 0; n < sizeof(bytes); ++n) {
        assert(analytics_decode_record(&decoded, bytes, n) == -EBADMSG);
        memcpy(corrupt, bytes, sizeof(bytes));
        corrupt[n] ^= 1;
        assert(analytics_decode_record(&decoded, corrupt, sizeof(corrupt)) == -EBADMSG);
    }

    struct analytics_saved_record old = {0};
    analytics_poc_init(&old.stats, NULL);
    old.generation = 5;
    old.stats.presses[0][0] = 7;
    old.stats.presses[41][1] = 9;
    old.stats.releases[0] = 3;
    old.stats.releases[1] = 4;
    old.stats.invalid_positions = 2;
    old.stats.saturated = true;
    old.layout_hash[0] = 0x5A;
    make_v1(v1, &old);
    assert(analytics_decode_record(&decoded, v1, sizeof(v1)) == 0);
    assert(decoded.generation == 5 && decoded.schema_version == 1);
    assert(decoded.layout_hash[0] == 0x5A && decoded.stats.saturated);
    assert(decoded.stats.presses[0][0] == 7 && decoded.stats.presses[41][1] == 9);
    assert(decoded.stats.releases[0] == 3 && decoded.stats.releases[1] == 4);
    assert(decoded.stats.invalid_positions == 2 && decoded.stats.pair_count == 0);
    assert(decoded.stats.hold[0][0] == 0 && decoded.stats.duplicate_downs == 0);
    memcpy(corrupt, v1, sizeof(v1));
    corrupt[100] ^= 1;
    assert(analytics_decode_record(&decoded, corrupt, sizeof(v1)) == -EBADMSG);

    states[0] = ANALYTICS_SLOT_VALID;
    assert(analytics_choose_record(records, states, identity) == -EXDEV);
    memcpy(records[0].layout_hash, identity, 32);
    assert(analytics_choose_record(records, states, identity) == 0);
    states[1] = ANALYTICS_SLOT_BAD;
    assert(analytics_choose_record(records, states, identity) == 0);
    states[0] = ANALYTICS_SLOT_BAD;
    assert(analytics_choose_record(records, states, identity) == -EBADMSG);
    states[0] = states[1] = ANALYTICS_SLOT_VALID;
    records[1] = records[0];
    records[1].generation = 2;
    assert(analytics_choose_record(records, states, identity) == 1);
    records[1] = records[0];
    records[1].stats.presses[0][0]++;
    assert(analytics_choose_record(records, states, identity) == -EILSEQ);
    records[1] = records[0];
    records[1].generation = UINT32_MAX;
    assert(analytics_choose_record(records, states, identity) == 1);

    struct analytics_poc_snapshot early = {0};
    analytics_poc_init(&early, "1:0");
    early.presses[0][0] = 3;
    early.presses[41][1] = 10;
    early.hold[3][1] = 1;
    early.pair_near[0][ANALYTICS_TIMING_CC][1] = 2;
    analytics_poc_merge(&early, &records[0].stats);
    assert(early.presses[0][0] == 0x1234567B);
    assert(early.presses[41][1] == UINT32_MAX && early.saturated);
    assert(early.releases[0] == 7 && early.invalid_positions == 9);
    assert(early.duplicate_downs == 2 && early.orphan_releases == 3);
    assert(early.hold[3][1] == 1 && early.hold[3][2] == 11);
    assert(early.pair_near[0][ANALYTICS_TIMING_CC][1] == 16);
    assert(early.pair_count == 1 && early.pairs[0][0] == 0 && early.pairs[0][1] == 1);
    assert(records[0].stats.pair_count == 1 && records[0].stats.pairs[0][0] == 0);

    struct analytics_poc_snapshot dropped = {0};
    analytics_poc_init(&dropped, "0:1");
    analytics_poc_init(&early, "1:2");
    early.pair_near[0][ANALYTICS_TIMING_CC][0] = 9;
    analytics_poc_merge(&dropped, &early);
    assert(dropped.pair_count == 1 && dropped.pair_near[0][ANALYTICS_TIMING_CC][0] == 0);
    assert(early.pair_count == 1 && early.pairs[0][0] == 0 && early.pairs[0][1] == 1);

    struct analytics_saved_bigrams bigrams[2] = {0}, decoded_bigrams;
    enum analytics_slot_state bigram_states[2] = {ANALYTICS_SLOT_EMPTY, ANALYTICS_SLOT_EMPTY};
    uint8_t bigram_bytes[ANALYTICS_BIGRAM_RECORD_SIZE], bigram_corrupt[ANALYTICS_BIGRAM_RECORD_SIZE];
    analytics_bigrams_init(&bigrams[0].counts);
    bigrams[0].generation = 3;
    bigrams[0].layout_hash[0] = 0x77;
    bigrams[0].counts.counts[0][1] = 5;
    bigrams[0].counts.counts[1][0] = 7;
    bigrams[0].counts.counts[41][41] = 9;
    analytics_encode_bigram_record(bigram_bytes, &bigrams[0]);
    assert(analytics_decode_bigram_record(&decoded_bigrams, bigram_bytes,
                                          sizeof(bigram_bytes)) == 0);
    assert(decoded_bigrams.generation == 3 && decoded_bigrams.schema_version == 1);
    assert(decoded_bigrams.layout_hash[0] == 0x77);
    assert(analytics_bigrams_equal(&decoded_bigrams.counts, &bigrams[0].counts));
    for (size_t n = 0; n < sizeof(bigram_bytes); ++n) {
        assert(analytics_decode_bigram_record(&decoded_bigrams, bigram_bytes, n) == -EBADMSG);
        memcpy(bigram_corrupt, bigram_bytes, sizeof(bigram_bytes));
        bigram_corrupt[n] ^= 1;
        assert(analytics_decode_bigram_record(&decoded_bigrams, bigram_corrupt,
                                              sizeof(bigram_corrupt)) == -EBADMSG);
    }
    assert(analytics_choose_bigram_record(bigrams, bigram_states, identity) == -ENOENT);
    bigram_states[0] = ANALYTICS_SLOT_VALID;
    assert(analytics_choose_bigram_record(bigrams, bigram_states, identity) == -EXDEV);
    memcpy(bigrams[0].layout_hash, identity, 32);
    assert(analytics_choose_bigram_record(bigrams, bigram_states, identity) == 0);
    bigram_states[1] = ANALYTICS_SLOT_VALID;
    bigrams[1] = bigrams[0];
    bigrams[1].generation = 4;
    assert(analytics_choose_bigram_record(bigrams, bigram_states, identity) == 1);
    bigrams[1].counts.counts[0][1]++;
    bigrams[0].generation = 4;
    assert(analytics_choose_bigram_record(bigrams, bigram_states, identity) == -EILSEQ);

    struct analytics_bigram_counts merged, saved;
    analytics_bigrams_init(&merged);
    analytics_bigrams_init(&saved);
    merged.counts[0][0] = UINT16_MAX;
    saved.counts[0][1] = 4;
    analytics_bigrams_merge(&merged, &saved);
    assert(merged.counts[0][1] == 4);
    assert(merged.counts[0][0] == UINT16_MAX && merged.saturated);
    assert(!saved.saturated);
    assert(!analytics_bigrams_equal(&merged, &saved));

    puts("PASS: v2/v1 roundtrip, corruption, pair remap, bigram record");
    return 0;
}
