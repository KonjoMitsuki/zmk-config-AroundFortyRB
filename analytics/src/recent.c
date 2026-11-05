/* SPDX-License-Identifier: MIT */
#include <string.h>
#include <analytics/recent.h>

void analytics_recent_init(struct analytics_recent_log *log) {
    memset(log, 0, sizeof(*log));
}

void analytics_recent_record(struct analytics_recent_log *log, uint32_t position,
                             uint8_t source, int64_t timestamp) {
    if (position >= ANALYTICS_POSITION_COUNT) {
        return;
    }
    ++log->next_sequence;
    struct analytics_recent_event *event = &log->events[log->head];
    event->sequence = log->next_sequence;
    event->timestamp_ms =
        timestamp < 0 ? 0 : timestamp > (int64_t)UINT32_MAX ? UINT32_MAX
                                                            : (uint32_t)timestamp;
    event->position = (uint16_t)position;
    event->source = source == UINT8_MAX ? ANALYTICS_LOCAL : ANALYTICS_PERIPHERAL;
    event->reserved = 0;
    log->head = (uint16_t)((log->head + 1) % ANALYTICS_RECENT_CAPACITY);
    if (log->count < ANALYTICS_RECENT_CAPACITY) {
        ++log->count;
    }
}

const struct analytics_recent_event *analytics_recent_get(
    const struct analytics_recent_log *log, unsigned int index) {
    if (index >= log->count) {
        return NULL;
    }
    unsigned int start =
        (log->head + ANALYTICS_RECENT_CAPACITY - log->count) % ANALYTICS_RECENT_CAPACITY;
    return &log->events[(start + index) % ANALYTICS_RECENT_CAPACITY];
}
