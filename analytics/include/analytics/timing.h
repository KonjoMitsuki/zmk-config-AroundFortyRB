/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <analytics/collector.h>

struct analytics_timing_position {
    int64_t down_time;
    int64_t last_down_time;
    bool is_down;
    bool last_down_valid;
};

struct analytics_timing_state {
    struct analytics_timing_position positions[ANALYTICS_POSITION_COUNT];
    int64_t last_any_down_time;
    bool last_any_down_valid;
    uint32_t previous_position;
    int64_t previous_down_time;
    uint8_t previous_class;
    bool previous_valid;
};

struct analytics_timing_result {
    bool new_press;
    bool previous_valid;
    uint32_t previous_position;
};

void analytics_timing_init(struct analytics_timing_state *state);

/* Caller serializes access and has already validated the position for
 * analytics_poc_record(). Timestamps are event arrival times on the Central. */
struct analytics_timing_result analytics_timing_record(
    struct analytics_poc_snapshot *stats, struct analytics_timing_state *state,
    uint32_t position, uint8_t source, bool pressed, int64_t timestamp);
