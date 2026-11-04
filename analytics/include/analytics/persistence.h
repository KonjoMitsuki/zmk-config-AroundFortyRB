/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <analytics/collector.h>

/* Canonical little-endian record, no C struct padding. CRC-32/ISO-HDLC. */
#define ANALYTICS_RECORD_SIZE (72 + 8 * ANALYTICS_POSITION_COUNT)
#define ANALYTICS_LAYOUT_HASH_SIZE 32

struct analytics_saved_record {
    struct analytics_poc_snapshot stats;
    uint32_t generation;
    uint8_t layout_hash[ANALYTICS_LAYOUT_HASH_SIZE];
};

enum analytics_slot_state { ANALYTICS_SLOT_EMPTY, ANALYTICS_SLOT_VALID, ANALYTICS_SLOT_BAD };

void analytics_encode_record(uint8_t *bytes, const struct analytics_saved_record *record);
int analytics_decode_record(struct analytics_saved_record *record, const uint8_t *bytes, size_t size);
/* Select newest CRC-valid record, then check identity. Never fall back across layouts. */
int analytics_choose_record(const struct analytics_saved_record records[2],
                            const enum analytics_slot_state slots[2], const uint8_t layout_hash[32]);
bool analytics_stats_equal(const struct analytics_poc_snapshot *a,
                           const struct analytics_poc_snapshot *b);

struct analytics_storage_status {
    const char *state;
    uint32_t generation;
    uint32_t error_count_session;
    int last_error;
    bool enabled;
    bool dirty;
    bool recovered;
};

/* These may wait for a save: diagnostics/worker threads only, never key callbacks. */
void zmk_analytics_capture(struct analytics_poc_snapshot *stats, struct analytics_storage_status *status);
int zmk_analytics_save(void);
