/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <analytics/collector.h>

/* Host simulator only. Runs after the synthetic kscan script exits, outside the
 * event hot path. Production firmware has neither this code nor this output. */
static void report(void) {
    struct analytics_poc_snapshot snapshot;
    zmk_analytics_poc_copy(&snapshot);
    for (unsigned int i = 0; i < ANALYTICS_POSITION_COUNT; ++i) {
        printf("ANALYTICS_POC position=%u local=%u peripheral=%u\n", i,
               snapshot.presses[i][ANALYTICS_LOCAL], snapshot.presses[i][ANALYTICS_PERIPHERAL]);
    }
    printf("ANALYTICS_POC releases_local=%u releases_peripheral=%u invalid=%u saturated=%u\n",
           snapshot.releases[ANALYTICS_LOCAL], snapshot.releases[ANALYTICS_PERIPHERAL],
           snapshot.invalid_positions, snapshot.saturated);
}

static int register_report(void) { return atexit(report); }

SYS_INIT(register_report, APPLICATION, 99);
