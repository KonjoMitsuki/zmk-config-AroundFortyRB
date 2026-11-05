/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <analytics/bigram.h>
#include <analytics/collector.h>
#include <analytics/recent.h>
#include <analytics/timing.h>

static void reset(struct analytics_poc_snapshot *stats, struct analytics_timing_state *timing,
                  const char *pairs) {
    analytics_poc_init(stats, pairs);
    analytics_timing_init(timing);
}

int main(void) {
    struct analytics_poc_snapshot stats;
    struct analytics_timing_state timing;
    struct analytics_timing_result result;

    reset(&stats, &timing, "0:1,1:2");
    result = analytics_timing_record(&stats, &timing, 0, UINT8_MAX, true, 1000);
    assert(result.new_press && !result.previous_valid);
    analytics_timing_record(&stats, &timing, 0, UINT8_MAX, false, 1030);
    assert(stats.hold[0][1] == 1);

    result = analytics_timing_record(&stats, &timing, 0, UINT8_MAX, true, 2000);
    assert(result.new_press && result.previous_valid && result.previous_position == 0);
    assert(stats.same_key[0][7] == 1);
    assert(stats.idle_before_press[0][7] == 1);

    result = analytics_timing_record(&stats, &timing, 1, UINT8_MAX, true, 2050);
    assert(result.new_press && result.previous_position == 0);
    assert(stats.idle_before_press[1][1] == 1);
    assert(stats.pair_near[0][ANALYTICS_TIMING_CC][2] == 1);
    assert(stats.pair_overlap[0][ANALYTICS_TIMING_CC][2] == 1);
    analytics_timing_record(&stats, &timing, 1, UINT8_MAX, false, 2100);
    assert(stats.hold[1][2] == 1);
    analytics_timing_record(&stats, &timing, 0, UINT8_MAX, false, 2500);
    assert(stats.hold[0][7] == 1);

    reset(&stats, &timing, "0:1");
    analytics_timing_record(&stats, &timing, 2, UINT8_MAX, true, 3000);
    result = analytics_timing_record(&stats, &timing, 2, UINT8_MAX, true, 3001);
    assert(!result.new_press && stats.duplicate_downs == 1);
    analytics_timing_record(&stats, &timing, 2, UINT8_MAX, false, 3010);
    assert(stats.hold[2][0] == 1);

    result = analytics_timing_record(&stats, &timing, 3, UINT8_MAX, false, 4000);
    assert(!result.new_press && stats.orphan_releases == 1);

    analytics_timing_record(&stats, &timing, 4, UINT8_MAX, true, 5000);
    analytics_timing_record(&stats, &timing, 4, UINT8_MAX, false, 4990);
    assert(stats.timing_discards == 1 && stats.hold[4][0] == 0);

    reset(&stats, &timing, "5:6");
    analytics_timing_record(&stats, &timing, 5, UINT8_MAX, true, 6000);
    analytics_timing_record(&stats, &timing, 6, UINT8_MAX, true, 6250);
    assert(stats.pair_near[0][ANALYTICS_TIMING_CC][0] == 0);
    assert(stats.pair_overlap[0][ANALYTICS_TIMING_CC][0] == 0);

    reset(&stats, &timing, "0:1");
    analytics_timing_record(&stats, &timing, 0, 0, true, 7000);
    analytics_timing_record(&stats, &timing, 1, 0, true, 7020);
    assert(stats.pair_near[0][ANALYTICS_TIMING_PP][1] == 1);
    assert(stats.pair_overlap[0][ANALYTICS_TIMING_PP][1] == 1);

    reset(&stats, &timing, "0:1");
    analytics_timing_record(&stats, &timing, 0, UINT8_MAX, true, 8000);
    analytics_timing_record(&stats, &timing, 1, 0, true, 8010);
    assert(stats.pair_near[0][ANALYTICS_TIMING_CP][0] == 1);
    assert(stats.pair_overlap[0][ANALYTICS_TIMING_CP][0] == 1);

    reset(&stats, &timing, "0:1");
    analytics_timing_record(&stats, &timing, 0, UINT8_MAX, true, 0);
    stats.hold[0][0] = UINT16_MAX;
    analytics_timing_record(&stats, &timing, 0, UINT8_MAX, false, 10);
    assert(stats.hold[0][0] == UINT16_MAX && stats.histogram_saturated);

    reset(&stats, &timing, "0:1,2:3,1:0,2:2,5,42:0");
    assert(stats.pair_count == 2);
    assert(stats.pairs[0][0] == 0 && stats.pairs[0][1] == 1);
    assert(stats.pairs[1][0] == 2 && stats.pairs[1][1] == 3);
    assert(stats.pair_config_errors == 3);

    struct analytics_bigram_counts bigrams, saved;
    analytics_bigrams_init(&bigrams);
    analytics_bigram_record(&bigrams, 0, 1);
    analytics_bigram_record(&bigrams, 1, 0);
    analytics_bigram_record(&bigrams, 1, 0);
    assert(bigrams.counts[0][1] == 1 && bigrams.counts[1][0] == 2);
    analytics_bigrams_init(&saved);
    saved.counts[0][1] = UINT16_MAX - 1;
    analytics_bigrams_merge(&bigrams, &saved);
    assert(bigrams.counts[0][1] == UINT16_MAX && bigrams.saturated);

    struct analytics_recent_log recent;
    analytics_recent_init(&recent);
    for (unsigned int i = 0; i < ANALYTICS_RECENT_CAPACITY + 3; ++i) {
        analytics_recent_record(&recent, i % ANALYTICS_POSITION_COUNT,
                                i % 2 ? 0 : UINT8_MAX, i * 10);
    }
    assert(recent.count == ANALYTICS_RECENT_CAPACITY);
    assert(recent.next_sequence == ANALYTICS_RECENT_CAPACITY + 3);
    const struct analytics_recent_event *first = analytics_recent_get(&recent, 0);
    const struct analytics_recent_event *last =
        analytics_recent_get(&recent, ANALYTICS_RECENT_CAPACITY - 1);
    assert(first && first->sequence == 4);
    assert(last && last->sequence == ANALYTICS_RECENT_CAPACITY + 3);
    assert(analytics_recent_get(&recent, ANALYTICS_RECENT_CAPACITY) == NULL);
    analytics_recent_init(&recent);
    analytics_recent_record(&recent, 0, UINT8_MAX, -5);
    analytics_recent_record(&recent, 1, 0, (int64_t)UINT32_MAX + 5);
    assert(analytics_recent_get(&recent, 0)->timestamp_ms == 0);
    assert(analytics_recent_get(&recent, 1)->timestamp_ms == UINT32_MAX);
    assert(analytics_recent_get(&recent, 0)->source == ANALYTICS_LOCAL);
    assert(analytics_recent_get(&recent, 1)->source == ANALYTICS_PERIPHERAL);

    puts("PASS: timing bins, classes, duplicates, orphans, saturation, bigrams, recent");
    return 0;
}
