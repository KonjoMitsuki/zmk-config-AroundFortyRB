/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/events/position_state_changed.h>
#include <analytics/collector.h>

extern const struct zmk_listener zmk_listener_keymap;

static void probe(struct k_work *work) {
    ARG_UNUSED(work);
    /* Equal timestamps are intentional: clock values are not event identities.
     * Both sources use the real generated event function and linker wrapper. */
    for (unsigned int source = 0; source < 2; ++source) {
        for (unsigned int press = 0; press < 2; ++press) {
            struct zmk_position_state_changed data = {
                .position = source,
                .source = source == 0 ? UINT8_MAX : 0,
                .state = true,
                .timestamp = 7,
            };
            raise_zmk_position_state_changed(data);
            struct zmk_position_state_changed_event replay = {
                .header = {.event = &zmk_event_zmk_position_state_changed},
                .data = data,
            };
            ZMK_EVENT_RAISE(replay);
            ZMK_EVENT_RAISE_AT(replay, keymap);
            ZMK_EVENT_RAISE_AFTER(replay, keymap);
            ZMK_EVENT_RELEASE(replay);
            data.state = false;
            raise_zmk_position_state_changed(data);
            replay.data = data;
            ZMK_EVENT_RAISE(replay);
        }
    }

    struct analytics_poc_snapshot stats;
    zmk_analytics_poc_copy(&stats);
    __ASSERT(stats.presses[0][ANALYTICS_LOCAL] == 2, "local counted more/less than once");
    __ASSERT(stats.presses[1][ANALYTICS_PERIPHERAL] == 2, "peripheral source miscount");
    __ASSERT(stats.presses[0][ANALYTICS_PERIPHERAL] == 0, "wrong source class");
    __ASSERT(stats.presses[1][ANALYTICS_LOCAL] == 0, "wrong source class");
    __ASSERT(stats.releases[ANALYTICS_LOCAL] == 2, "local release replay counted");
    __ASSERT(stats.releases[ANALYTICS_PERIPHERAL] == 2, "remote release replay counted");
    __ASSERT(stats.invalid_positions == 0 && !stats.saturated, "unexpected quality flags");
    puts("PASS: new local/peripheral events, equal timestamps, RAISE/RELEASE/AT/AFTER replay");
    exit(0);
}

K_WORK_DELAYABLE_DEFINE(probe_work, probe);

static int start_probe(void) {
    k_work_schedule(&probe_work, K_MSEC(1));
    return 0;
}

SYS_INIT(start_probe, APPLICATION, 99);
