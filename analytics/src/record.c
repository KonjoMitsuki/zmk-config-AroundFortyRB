/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <string.h>
#include <analytics/persistence.h>

#define MAGIC_V1 0x31414B5Au /* ZKA1 */
#define MAGIC_V2 0x32414B5Au /* ZKA2 */
#define MAGIC_BIGRAM 0x32424B5Au /* ZKB2 */

static uint8_t get8(const uint8_t *p) { return p[0]; }

static uint16_t get16(const uint8_t *p) {
    return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}

static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void put8(uint8_t *p, uint32_t n) { p[0] = (uint8_t)n; }

static void put16(uint8_t *p, uint32_t n) {
    p[0] = (uint8_t)n;
    p[1] = (uint8_t)(n >> 8);
}

static void put32(uint8_t *p, uint32_t n) {
    for (unsigned int i = 0; i < 4; ++i) {
        p[i] = (uint8_t)(n >> (i * 8));
    }
}

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

void analytics_encode_record(uint8_t *bytes, const struct analytics_saved_record *record) {
    put32(bytes, MAGIC_V2);
    put32(bytes + 4, 2);
    put32(bytes + 8, ANALYTICS_RECORD_SIZE);
    put32(bytes + 12, ANALYTICS_POSITION_COUNT);
    put32(bytes + 16, record->generation);
    memcpy(bytes + 20, record->layout_hash, ANALYTICS_LAYOUT_HASH_SIZE);
    put32(bytes + 52, record->stats.pair_count);
    put32(bytes + 56, (record->stats.saturated ? 1u : 0u) |
                          (record->stats.histogram_saturated ? 2u : 0u));
    put32(bytes + 60, 0);
    size_t offset = 64;
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i, offset += 2) {
        put8(bytes + offset, record->stats.pairs[i][0]);
        put8(bytes + offset + 1, record->stats.pairs[i][1]);
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
            put32(bytes + offset, record->stats.presses[p][s]);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
        put32(bytes + offset, record->stats.releases[s]);
    }
    put32(bytes + offset, record->stats.invalid_positions);
    offset += 4;
    put32(bytes + offset, record->stats.duplicate_downs);
    offset += 4;
    put32(bytes + offset, record->stats.orphan_releases);
    offset += 4;
    put32(bytes + offset, record->stats.timing_discards);
    offset += 4;
    put32(bytes + offset, record->stats.pair_config_errors);
    offset += 4;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            put16(bytes + offset, record->stats.hold[p][b]);
        }
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            put16(bytes + offset, record->stats.same_key[p][b]);
        }
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            put16(bytes + offset, record->stats.idle_before_press[p][b]);
        }
    }
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i) {
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            for (unsigned int b = 0; b < ANALYTICS_PAIR_BINS; ++b, offset += 2) {
                put16(bytes + offset, record->stats.pair_near[i][c][b]);
            }
        }
    }
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i) {
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            for (unsigned int b = 0; b < ANALYTICS_PAIR_BINS; ++b, offset += 2) {
                put16(bytes + offset, record->stats.pair_overlap[i][c][b]);
            }
        }
    }
    put32(bytes + ANALYTICS_RECORD_SIZE - 4, crc32(bytes, ANALYTICS_RECORD_SIZE - 4));
}

static int decode_v1(struct analytics_saved_record *record, const uint8_t *bytes, size_t size) {
    if (get32(bytes + 8) != size || get32(bytes + 12) != ANALYTICS_POSITION_COUNT ||
        get32(bytes + 16) == 0 || get32(bytes + 52) > 1 ||
        get32(bytes + size - 4) != crc32(bytes, size - 4)) {
        return -EBADMSG;
    }
    memset(record, 0, sizeof(*record));
    record->generation = get32(bytes + 16);
    record->schema_version = 1;
    memcpy(record->layout_hash, bytes + 20, ANALYTICS_LAYOUT_HASH_SIZE);
    record->stats.saturated = get32(bytes + 52) != 0;
    size_t offset = 56;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
            record->stats.presses[p][s] = get32(bytes + offset);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
        record->stats.releases[s] = get32(bytes + offset);
    }
    record->stats.invalid_positions = get32(bytes + offset);
    return 0;
}

static int decode_v2(struct analytics_saved_record *record, const uint8_t *bytes, size_t size) {
    uint32_t pair_count = get32(bytes + 52);
    uint32_t flags = get32(bytes + 56);
    if (get32(bytes + 8) != size || get32(bytes + 12) != ANALYTICS_POSITION_COUNT ||
        get32(bytes + 16) == 0 || pair_count > ANALYTICS_PAIR_MAX || flags > 3 ||
        get32(bytes + 60) != 0 ||
        get32(bytes + size - 4) != crc32(bytes, size - 4)) {
        return -EBADMSG;
    }
    memset(record, 0, sizeof(*record));
    record->generation = get32(bytes + 16);
    record->schema_version = 2;
    memcpy(record->layout_hash, bytes + 20, ANALYTICS_LAYOUT_HASH_SIZE);
    record->stats.pair_count = (uint8_t)pair_count;
    record->stats.saturated = (flags & 1u) != 0;
    record->stats.histogram_saturated = (flags & 2u) != 0;
    size_t offset = 64;
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i, offset += 2) {
        record->stats.pairs[i][0] = get8(bytes + offset);
        record->stats.pairs[i][1] = get8(bytes + offset + 1);
    }
    for (unsigned int i = 0; i < pair_count; ++i) {
        if (record->stats.pairs[i][0] >= ANALYTICS_POSITION_COUNT ||
            record->stats.pairs[i][1] >= ANALYTICS_POSITION_COUNT ||
            record->stats.pairs[i][0] == record->stats.pairs[i][1]) {
            return -EBADMSG;
        }
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
            record->stats.presses[p][s] = get32(bytes + offset);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s, offset += 4) {
        record->stats.releases[s] = get32(bytes + offset);
    }
    record->stats.invalid_positions = get32(bytes + offset);
    offset += 4;
    record->stats.duplicate_downs = get32(bytes + offset);
    offset += 4;
    record->stats.orphan_releases = get32(bytes + offset);
    offset += 4;
    record->stats.timing_discards = get32(bytes + offset);
    offset += 4;
    record->stats.pair_config_errors = get32(bytes + offset);
    offset += 4;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            record->stats.hold[p][b] = get16(bytes + offset);
        }
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            record->stats.same_key[p][b] = get16(bytes + offset);
        }
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b, offset += 2) {
            record->stats.idle_before_press[p][b] = get16(bytes + offset);
        }
    }
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i) {
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            for (unsigned int b = 0; b < ANALYTICS_PAIR_BINS; ++b, offset += 2) {
                record->stats.pair_near[i][c][b] = get16(bytes + offset);
            }
        }
    }
    for (unsigned int i = 0; i < ANALYTICS_PAIR_MAX; ++i) {
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            for (unsigned int b = 0; b < ANALYTICS_PAIR_BINS; ++b, offset += 2) {
                record->stats.pair_overlap[i][c][b] = get16(bytes + offset);
            }
        }
    }
    return 0;
}

int analytics_decode_record(struct analytics_saved_record *record, const uint8_t *bytes,
                            size_t size) {
    if (size < 8) {
        return -EBADMSG;
    }
    uint32_t magic = get32(bytes);
    uint32_t version = get32(bytes + 4);
    if (magic == MAGIC_V1 && version == 1 && size == ANALYTICS_RECORD_SIZE_V1) {
        return decode_v1(record, bytes, size);
    }
    if (magic == MAGIC_V2 && version == 2 && size == ANALYTICS_RECORD_SIZE) {
        return decode_v2(record, bytes, size);
    }
    return -EBADMSG;
}

void analytics_encode_bigram_record(uint8_t *bytes,
                                     const struct analytics_saved_bigrams *record) {
    put32(bytes, MAGIC_BIGRAM);
    put32(bytes + 4, 1);
    put32(bytes + 8, ANALYTICS_BIGRAM_RECORD_SIZE);
    put32(bytes + 12, ANALYTICS_POSITION_COUNT);
    put32(bytes + 16, record->generation);
    memcpy(bytes + 20, record->layout_hash, ANALYTICS_LAYOUT_HASH_SIZE);
    put32(bytes + 52, record->counts.saturated ? 1u : 0u);
    put32(bytes + 56, 0);
    put32(bytes + 60, 0);
    size_t offset = 64;
    for (unsigned int a = 0; a < ANALYTICS_POSITION_COUNT; ++a) {
        for (unsigned int b = 0; b < ANALYTICS_POSITION_COUNT; ++b, offset += 2) {
            put16(bytes + offset, record->counts.counts[a][b]);
        }
    }
    put32(bytes + ANALYTICS_BIGRAM_RECORD_SIZE - 4,
          crc32(bytes, ANALYTICS_BIGRAM_RECORD_SIZE - 4));
}

int analytics_decode_bigram_record(struct analytics_saved_bigrams *record, const uint8_t *bytes,
                                   size_t size) {
    if (size != ANALYTICS_BIGRAM_RECORD_SIZE || get32(bytes) != MAGIC_BIGRAM ||
        get32(bytes + 4) != 1 || get32(bytes + 8) != size ||
        get32(bytes + 12) != ANALYTICS_POSITION_COUNT || get32(bytes + 16) == 0 ||
        get32(bytes + 52) > 1 || get32(bytes + 56) != 0 || get32(bytes + 60) != 0 ||
        get32(bytes + size - 4) != crc32(bytes, size - 4)) {
        return -EBADMSG;
    }
    memset(record, 0, sizeof(*record));
    record->generation = get32(bytes + 16);
    record->schema_version = 1;
    memcpy(record->layout_hash, bytes + 20, ANALYTICS_LAYOUT_HASH_SIZE);
    record->counts.saturated = get32(bytes + 52) != 0;
    size_t offset = 64;
    for (unsigned int a = 0; a < ANALYTICS_POSITION_COUNT; ++a) {
        for (unsigned int b = 0; b < ANALYTICS_POSITION_COUNT; ++b, offset += 2) {
            record->counts.counts[a][b] = get16(bytes + offset);
        }
    }
    return 0;
}

int analytics_choose_record(const struct analytics_saved_record records[2],
                            const enum analytics_slot_state slots[2],
                            const uint8_t layout_hash[32]) {
    int best = -1;
    for (int slot = 0; slot < 2; ++slot) {
        if (slots[slot] == ANALYTICS_SLOT_VALID &&
            (best < 0 || records[slot].generation > records[best].generation)) {
            best = slot;
        }
    }
    if (best < 0) {
        return slots[0] == ANALYTICS_SLOT_BAD || slots[1] == ANALYTICS_SLOT_BAD ? -EBADMSG : -ENOENT;
    }
    if (slots[0] == ANALYTICS_SLOT_VALID && slots[1] == ANALYTICS_SLOT_VALID &&
        records[0].generation == records[1].generation &&
        (memcmp(records[0].layout_hash, records[1].layout_hash, 32) != 0 ||
         !analytics_stats_equal(&records[0].stats, &records[1].stats))) {
        return -EILSEQ;
    }
    return memcmp(records[best].layout_hash, layout_hash, 32) == 0 ? best : -EXDEV;
}

int analytics_choose_bigram_record(const struct analytics_saved_bigrams records[2],
                                   const enum analytics_slot_state slots[2],
                                   const uint8_t layout_hash[32]) {
    int best = -1;
    for (int slot = 0; slot < 2; ++slot) {
        if (slots[slot] == ANALYTICS_SLOT_VALID &&
            (best < 0 || records[slot].generation > records[best].generation)) {
            best = slot;
        }
    }
    if (best < 0) {
        return slots[0] == ANALYTICS_SLOT_BAD || slots[1] == ANALYTICS_SLOT_BAD ? -EBADMSG : -ENOENT;
    }
    if (slots[0] == ANALYTICS_SLOT_VALID && slots[1] == ANALYTICS_SLOT_VALID &&
        records[0].generation == records[1].generation &&
        (memcmp(records[0].layout_hash, records[1].layout_hash, 32) != 0 ||
         !analytics_bigrams_equal(&records[0].counts, &records[1].counts))) {
        return -EILSEQ;
    }
    return memcmp(records[best].layout_hash, layout_hash, 32) == 0 ? best : -EXDEV;
}
