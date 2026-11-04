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

enum analytics_source_class {
    ANALYTICS_LOCAL,
    ANALYTICS_PERIPHERAL,
    ANALYTICS_SOURCE_CLASSES,
};

/* Source-event counts, not confirmed HID output or physical timing measurements.
 * Peripheral releases can be synthetic disconnect cleanup. No event history. */
struct analytics_poc_snapshot {
    uint32_t presses[ANALYTICS_POSITION_COUNT][ANALYTICS_SOURCE_CLASSES];
    uint32_t releases[ANALYTICS_SOURCE_CLASSES];
    uint32_t invalid_positions;
    bool saturated;
};

/* Caller serializes access. source == UINT8_MAX is local in ZMK v0.3.0. */
void analytics_poc_record(struct analytics_poc_snapshot *stats, uint32_t position,
                          uint8_t source, bool pressed);
void analytics_poc_merge(struct analytics_poc_snapshot *destination,
                         const struct analytics_poc_snapshot *saved);

/* In-firmware diagnostic API only; not a host-facing authorization boundary. */
void zmk_analytics_poc_copy(struct analytics_poc_snapshot *destination);
void zmk_analytics_poc_restore(const struct analytics_poc_snapshot *saved);
