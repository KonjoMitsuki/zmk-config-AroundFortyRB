/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef ANALYTICS_ZMK_BUILD
#include <zmk/matrix.h>
#define ANALYTICS_POSITION_COUNT ZMK_KEYMAP_LEN
#endif

#ifndef ANALYTICS_POSITION_COUNT
#error "Define ANALYTICS_POSITION_COUNT for a host test"
#endif

#define ANALYTICS_TIME_BINS 8
#define ANALYTICS_PAIR_BINS 6
#define ANALYTICS_PAIR_MAX 16
#define ANALYTICS_TIMING_CLASSES 3

enum analytics_source_class {
    ANALYTICS_LOCAL,
    ANALYTICS_PERIPHERAL,
    ANALYTICS_SOURCE_CLASSES,
};

enum analytics_timing_class {
    ANALYTICS_TIMING_CC,
    ANALYTICS_TIMING_PP,
    ANALYTICS_TIMING_CP,
};

/* Bounded source-event aggregates. Peripheral releases can be synthetic
 * disconnect cleanup. Timing is the event arrival time on the Central, not
 * the physical switch contact time. No per-press history is retained here. */
struct analytics_poc_snapshot {
    uint32_t presses[ANALYTICS_POSITION_COUNT][ANALYTICS_SOURCE_CLASSES];
    uint32_t releases[ANALYTICS_SOURCE_CLASSES];
    uint32_t invalid_positions;
    uint32_t duplicate_downs;
    uint32_t orphan_releases;
    uint32_t timing_discards;
    uint32_t pair_config_errors;
    bool saturated;
    bool histogram_saturated;
    uint8_t pair_count;
    uint8_t pairs[ANALYTICS_PAIR_MAX][2];

    uint16_t hold[ANALYTICS_POSITION_COUNT][ANALYTICS_TIME_BINS];
    uint16_t same_key[ANALYTICS_POSITION_COUNT][ANALYTICS_TIME_BINS];
    uint16_t idle_before_press[ANALYTICS_POSITION_COUNT][ANALYTICS_TIME_BINS];
    uint16_t pair_near[ANALYTICS_PAIR_MAX][ANALYTICS_TIMING_CLASSES][ANALYTICS_PAIR_BINS];
    uint16_t pair_overlap[ANALYTICS_PAIR_MAX][ANALYTICS_TIMING_CLASSES][ANALYTICS_PAIR_BINS];
};

/* Zeroes everything, then parses the comma-separated "a:b,c:d" pair list.
 * Invalid tokens and pairs beyond ANALYTICS_PAIR_MAX increment
 * pair_config_errors; valid pairs are normalized and deduplicated. */
void analytics_poc_init(struct analytics_poc_snapshot *stats, const char *pair_list);

/* Caller serializes access. source == UINT8_MAX is local in ZMK v0.3.0. */
void analytics_poc_record(struct analytics_poc_snapshot *stats, uint32_t position,
                          uint8_t source, bool pressed);

/* Adds saved to destination. Pair histograms in saved are remapped to the
 * destination's current pair list and saved is normalized in place. */
void analytics_poc_merge(struct analytics_poc_snapshot *destination,
                         struct analytics_poc_snapshot *saved);

bool analytics_stats_equal(const struct analytics_poc_snapshot *a,
                           const struct analytics_poc_snapshot *b);

/* In-firmware diagnostic API only; not a host-facing authorization boundary. */
void zmk_analytics_poc_copy(struct analytics_poc_snapshot *destination);
void zmk_analytics_poc_restore(struct analytics_poc_snapshot *saved);
