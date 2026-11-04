/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <analytics/collector.h>

int main(void) {
    struct analytics_poc_snapshot stats = {0};
    analytics_poc_record(&stats, 0, UINT8_MAX, true);
    analytics_poc_record(&stats, 0, UINT8_MAX, false);
    analytics_poc_record(&stats, 1, 0, true);
    analytics_poc_record(&stats, 1, 0, false);
    analytics_poc_record(&stats, 1, 1, true);
    assert(stats.presses[0][ANALYTICS_LOCAL] == 1);
    assert(stats.presses[1][ANALYTICS_PERIPHERAL] == 2);
    assert(stats.presses[0][ANALYTICS_PERIPHERAL] == 0);
    assert(stats.releases[ANALYTICS_LOCAL] == 1);
    assert(stats.releases[ANALYTICS_PERIPHERAL] == 1);

    analytics_poc_record(&stats, ANALYTICS_POSITION_COUNT, 0, true);
    analytics_poc_record(&stats, UINT32_MAX, UINT8_MAX, false);
    assert(stats.invalid_positions == 2);

    stats.presses[0][ANALYTICS_LOCAL] = UINT32_MAX - 1;
    analytics_poc_record(&stats, 0, UINT8_MAX, true);
    assert(stats.presses[0][ANALYTICS_LOCAL] == UINT32_MAX && stats.saturated);
    analytics_poc_record(&stats, 0, UINT8_MAX, true);
    assert(stats.presses[0][ANALYTICS_LOCAL] == UINT32_MAX);
    stats.releases[ANALYTICS_PERIPHERAL] = UINT32_MAX;
    analytics_poc_record(&stats, 1, 0, false);
    assert(stats.releases[ANALYTICS_PERIPHERAL] == UINT32_MAX);
    stats.invalid_positions = UINT32_MAX;
    analytics_poc_record(&stats, UINT32_MAX, 0, true);
    assert(stats.invalid_positions == UINT32_MAX);
    puts("PASS: source classification, release exclusion, bounds, saturation");
    return 0;
}
