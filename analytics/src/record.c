/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <string.h>
#include <analytics/persistence.h>

static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void put32(uint8_t *p, uint32_t n) {
    for (unsigned int i = 0; i < 4; ++i) { p[i] = n >> (i * 8); }
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
    put32(bytes, 0x31414B5Au); /* ZKA1 */
    put32(bytes + 4, 1);
    put32(bytes + 8, ANALYTICS_RECORD_SIZE);
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
    put32(bytes + ANALYTICS_RECORD_SIZE - 4, crc32(bytes, ANALYTICS_RECORD_SIZE - 4));
}

int analytics_decode_record(struct analytics_saved_record *record, const uint8_t *bytes, size_t size) {
    if (size != ANALYTICS_RECORD_SIZE || get32(bytes) != 0x31414B5Au || get32(bytes + 4) != 1 ||
        get32(bytes + 8) != size || get32(bytes + 12) != ANALYTICS_POSITION_COUNT ||
        get32(bytes + 16) == 0 || get32(bytes + 52) > 1 ||
        get32(bytes + size - 4) != crc32(bytes, size - 4)) {
        return -EBADMSG;
    }
    memset(record, 0, sizeof(*record));
    record->generation = get32(bytes + 16);
    memcpy(record->layout_hash, bytes + 20, 32);
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

bool analytics_stats_equal(const struct analytics_poc_snapshot *a,
                           const struct analytics_poc_snapshot *b) {
    return memcmp(a->presses, b->presses, sizeof(a->presses)) == 0 &&
           memcmp(a->releases, b->releases, sizeof(a->releases)) == 0 &&
           a->invalid_positions == b->invalid_positions && a->saturated == b->saturated;
}

int analytics_choose_record(const struct analytics_saved_record records[2],
                            const enum analytics_slot_state slots[2], const uint8_t layout_hash[32]) {
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
