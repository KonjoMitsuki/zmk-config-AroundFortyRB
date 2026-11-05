/* SPDX-License-Identifier: MIT */
#include <string.h>
#include <analytics/timing.h>

#define ANALYTICS_PAIR_WINDOW_MS 250

static const uint16_t hold_bounds[] = {20, 40, 60, 80, 100, 200, 500};
static const uint16_t interval_bounds[] = {50, 100, 150, 200, 300, 500, 1000};
static const uint16_t pair_bounds[] = {20, 40, 60, 80, 100};

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

static void add_hist(struct analytics_poc_snapshot *stats, uint16_t *value) {
    if (*value == UINT16_MAX) {
        stats->histogram_saturated = true;
    } else {
        ++*value;
        if (*value == UINT16_MAX) {
            stats->histogram_saturated = true;
        }
    }
}

static unsigned int bin_index(uint32_t milliseconds, const uint16_t *bounds,
                              unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        if (milliseconds < bounds[i]) {
            return i;
        }
    }
    return count;
}

static bool elapsed_ms(int64_t now, int64_t then, uint32_t *out) {
    if (now < then) {
        return false;
    }
    int64_t difference = now - then;
    *out = difference > (int64_t)UINT32_MAX ? UINT32_MAX : (uint32_t)difference;
    return true;
}

static int find_pair(const struct analytics_poc_snapshot *stats, uint32_t a, uint32_t b) {
    for (unsigned int i = 0; i < stats->pair_count; ++i) {
        if ((stats->pairs[i][0] == a && stats->pairs[i][1] == b) ||
            (stats->pairs[i][0] == b && stats->pairs[i][1] == a)) {
            return (int)i;
        }
    }
    return -1;
}

static enum analytics_source_class source_class(uint8_t source) {
    return source == UINT8_MAX ? ANALYTICS_LOCAL : ANALYTICS_PERIPHERAL;
}

static enum analytics_timing_class timing_class(enum analytics_source_class previous,
                                                enum analytics_source_class current) {
    if (previous == ANALYTICS_LOCAL && current == ANALYTICS_LOCAL) {
        return ANALYTICS_TIMING_CC;
    }
    if (previous == ANALYTICS_PERIPHERAL && current == ANALYTICS_PERIPHERAL) {
        return ANALYTICS_TIMING_PP;
    }
    return ANALYTICS_TIMING_CP;
}

void analytics_timing_init(struct analytics_timing_state *state) {
    memset(state, 0, sizeof(*state));
}

struct analytics_timing_result analytics_timing_record(
    struct analytics_poc_snapshot *stats, struct analytics_timing_state *state,
    uint32_t position, uint8_t source, bool pressed, int64_t timestamp) {
    struct analytics_timing_result result = {0};
    if (position >= ANALYTICS_POSITION_COUNT) {
        return result;
    }

    struct analytics_timing_position *position_state = &state->positions[position];
    enum analytics_source_class current_class = source_class(source);

    if (pressed) {
        if (position_state->is_down) {
            increment(stats, &stats->duplicate_downs);
            return result;
        }

        result.new_press = true;
        result.previous_valid = state->previous_valid;
        result.previous_position = state->previous_position;

        if (position_state->last_down_valid) {
            uint32_t gap;
            if (elapsed_ms(timestamp, position_state->last_down_time, &gap)) {
                add_hist(stats, &stats->same_key[position]
                                            [bin_index(gap, interval_bounds, 7)]);
            } else {
                increment(stats, &stats->timing_discards);
            }
        }
        if (state->last_any_down_valid) {
            uint32_t gap;
            if (elapsed_ms(timestamp, state->last_any_down_time, &gap)) {
                add_hist(stats, &stats->idle_before_press[position]
                                                     [bin_index(gap, interval_bounds, 7)]);
            } else {
                increment(stats, &stats->timing_discards);
            }
        }
        if (state->previous_valid && state->previous_position != position) {
            uint32_t gap;
            if (!elapsed_ms(timestamp, state->previous_down_time, &gap)) {
                increment(stats, &stats->timing_discards);
            } else if (gap < ANALYTICS_PAIR_WINDOW_MS) {
                int pair = find_pair(stats, state->previous_position, position);
                if (pair >= 0) {
                    enum analytics_timing_class kind =
                        timing_class(state->previous_class, current_class);
                    unsigned int bin = bin_index(gap, pair_bounds, ANALYTICS_PAIR_BINS);
                    add_hist(stats, &stats->pair_near[pair][kind][bin]);
                    if (state->positions[state->previous_position].is_down) {
                        add_hist(stats, &stats->pair_overlap[pair][kind][bin]);
                    }
                }
            }
        }

        position_state->is_down = true;
        position_state->down_time = timestamp;
        position_state->last_down_time = timestamp;
        position_state->last_down_valid = true;
        state->last_any_down_time = timestamp;
        state->last_any_down_valid = true;
        state->previous_position = position;
        state->previous_down_time = timestamp;
        state->previous_class = (uint8_t)current_class;
        state->previous_valid = true;
        return result;
    }

    if (!position_state->is_down) {
        increment(stats, &stats->orphan_releases);
        return result;
    }

    uint32_t duration;
    if (elapsed_ms(timestamp, position_state->down_time, &duration)) {
        add_hist(stats, &stats->hold[position][bin_index(duration, hold_bounds, 7)]);
    } else {
        increment(stats, &stats->timing_discards);
    }
    position_state->is_down = false;
    return result;
}
