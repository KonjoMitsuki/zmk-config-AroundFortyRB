/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <analytics/collector.h>

#ifndef ANALYTICS_RECENT_CAPACITY
#define ANALYTICS_RECENT_CAPACITY 1024
#endif

/* RAM-only recent press order. Not persisted; cleared on boot. The sequence
 * number gives an unambiguous order even when timestamps tie. */
struct analytics_recent_event {
    uint32_t sequence;
    uint32_t timestamp_ms;
    uint16_t position;
    uint8_t source;
    uint8_t reserved;
};

struct analytics_recent_log {
    uint32_t next_sequence;
    uint16_t head;
    uint16_t count;
    struct analytics_recent_event events[ANALYTICS_RECENT_CAPACITY];
};

void analytics_recent_init(struct analytics_recent_log *log);
void analytics_recent_record(struct analytics_recent_log *log, uint32_t position,
                             uint8_t source, int64_t timestamp);
/* index 0 is the oldest retained event; returns NULL when out of range. */
const struct analytics_recent_event *analytics_recent_get(
    const struct analytics_recent_log *log, unsigned int index);

/* Firmware diagnostic API; not a host-facing authorization boundary. */
void zmk_analytics_recent_copy(struct analytics_recent_log *destination);
