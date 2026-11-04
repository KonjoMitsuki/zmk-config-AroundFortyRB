/* SPDX-License-Identifier: MIT */
#include <analytics/collector.h>

static void increment(struct analytics_poc_snapshot *stats, uint32_t *counter) {
    if (*counter == UINT32_MAX) {
        stats->saturated = true;
    } else {
        ++*counter;
        if (*counter == UINT32_MAX) {
            stats->saturated = true;
        }
    }
}

void analytics_poc_record(struct analytics_poc_snapshot *stats, uint32_t position,
                          uint8_t source, bool pressed) {
    if (position >= ANALYTICS_POSITION_COUNT) {
        increment(stats, &stats->invalid_positions);
        return;
    }

    enum analytics_source_class kind =
        source == UINT8_MAX ? ANALYTICS_LOCAL : ANALYTICS_PERIPHERAL;
    increment(stats, pressed ? &stats->presses[position][kind] : &stats->releases[kind]);
}
