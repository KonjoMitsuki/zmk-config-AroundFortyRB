/* SPDX-License-Identifier: MIT */
#include <analytics/collector.h>

static uint32_t add(struct analytics_poc_snapshot *stats, uint32_t a, uint32_t b) {
    if (UINT32_MAX - a <= b) {
        stats->saturated = true;
        return UINT32_MAX;
    }
    return a + b;
}

void analytics_poc_merge(struct analytics_poc_snapshot *destination,
                         const struct analytics_poc_snapshot *saved) {
    destination->saturated |= saved->saturated;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s) {
            destination->presses[p][s] = add(destination, destination->presses[p][s], saved->presses[p][s]);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s) {
        destination->releases[s] = add(destination, destination->releases[s], saved->releases[s]);
    }
    destination->invalid_positions = add(destination, destination->invalid_positions,
                                         saved->invalid_positions);
}

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
