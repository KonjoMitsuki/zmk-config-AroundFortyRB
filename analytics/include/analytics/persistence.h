/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <analytics/bigram.h>
#include <analytics/collector.h>

/* Canonical little-endian records, no C struct padding. CRC-32/ISO-HDLC. */
#define ANALYTICS_RECORD_SIZE_V1 (72 + 8 * ANALYTICS_POSITION_COUNT)
/* 64-byte header + 32-byte pair list + counters + three position histograms
 * + two pair histogram families + CRC. */
#define ANALYTICS_RECORD_SIZE (1280 + 56 * ANALYTICS_POSITION_COUNT)
/* 64-byte header + directed bigram matrix + CRC. */
#define ANALYTICS_BIGRAM_RECORD_SIZE (68 + 2 * ANALYTICS_POSITION_COUNT * ANALYTICS_POSITION_COUNT)
#define ANALYTICS_LAYOUT_HASH_SIZE 32

struct analytics_saved_record {
    struct analytics_poc_snapshot stats;
    uint32_t generation;
    uint32_t schema_version;
    uint8_t layout_hash[ANALYTICS_LAYOUT_HASH_SIZE];
};

struct analytics_saved_bigrams {
    struct analytics_bigram_counts counts;
    uint32_t generation;
    uint32_t schema_version;
    uint8_t layout_hash[ANALYTICS_LAYOUT_HASH_SIZE];
};

enum analytics_slot_state { ANALYTICS_SLOT_EMPTY, ANALYTICS_SLOT_VALID, ANALYTICS_SLOT_BAD };

void analytics_encode_record(uint8_t *bytes, const struct analytics_saved_record *record);
int analytics_decode_record(struct analytics_saved_record *record, const uint8_t *bytes, size_t size);
void analytics_encode_bigram_record(uint8_t *bytes, const struct analytics_saved_bigrams *record);
int analytics_decode_bigram_record(struct analytics_saved_bigrams *record, const uint8_t *bytes,
                                   size_t size);
/* Select newest CRC-valid record, then check identity. Never fall back across layouts. */
int analytics_choose_record(const struct analytics_saved_record records[2],
                            const enum analytics_slot_state slots[2], const uint8_t layout_hash[32]);
int analytics_choose_bigram_record(const struct analytics_saved_bigrams records[2],
                                   const enum analytics_slot_state slots[2],
                                   const uint8_t layout_hash[32]);
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

/* Copies both aggregates under one lock so a saved pair is consistent even if
 * a key event arrives between the two. Diagnostics threads only. */
void zmk_analytics_snapshot_copy(struct analytics_poc_snapshot *stats,
                                 struct analytics_bigram_counts *bigrams);

/* These may wait for a save: diagnostics/worker threads only, never key callbacks. */
void zmk_analytics_capture(struct analytics_poc_snapshot *stats,
                           struct analytics_bigram_counts *bigrams,
                           struct analytics_storage_status *status);
int zmk_analytics_save(void);

/* Firmware diagnostic APIs; not host-facing authorization boundaries. */
void zmk_analytics_bigrams_copy(struct analytics_bigram_counts *destination);
void zmk_analytics_bigrams_restore(const struct analytics_bigram_counts *saved);
