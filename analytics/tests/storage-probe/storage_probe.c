/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zmk/events/position_state_changed.h>
#include <analytics/persistence.h>

static bool fail_write;
int __real_settings_save_one(const char *name, const void *data, size_t len);
int __wrap_settings_save_one(const char *name, const void *data, size_t len) {
    if (fail_write && strncmp(name, "analytics_poc/", 14) == 0) {
        return -EIO;
    }
    return __real_settings_save_one(name, data, len);
}

static void press(unsigned int position) {
    struct zmk_position_state_changed event = {.source = UINT8_MAX, .position = position,
                                               .state = true, .timestamp = k_uptime_get()};
    raise_zmk_position_state_changed(event);
    event.state = false;
    raise_zmk_position_state_changed(event);
}

static int check_marker(const char *key, size_t len, settings_read_cb cb, void *arg, void *param) {
    ARG_UNUSED(param);
    if (strcmp(key, "marker") == 0) {
        uint32_t value = 0;
        __ASSERT(len == sizeof(value) && cb(arg, &value, sizeof(value)) == sizeof(value) &&
                 value == 0x12345678, "unrelated settings damaged");
        *(bool *)param = true;
    }
    return 0;
}

static void probe(struct k_work *work) {
    ARG_UNUSED(work);
    const char *step = getenv("ANALYTICS_TEST_STEP");
    __ASSERT(step, "missing test step");
    struct analytics_poc_snapshot stats;
    struct analytics_bigram_counts bigrams;
    struct analytics_storage_status status;
    zmk_analytics_capture(&stats, &bigrams, &status);
    __ASSERT(strcmp(status.state, "loading") != 0, "settings not loaded");
    if (strcmp(step, "save") == 0) {
        __ASSERT(status.generation == 0 && stats.presses[0][0] == 1, "fresh boot");
        uint32_t marker = 0x12345678;
        __ASSERT(settings_save_one("unrelated/marker", &marker, sizeof(marker)) == 0, "marker save");
        press(1);
        __ASSERT(zmk_analytics_save() == 0, "save generation 1");
        press(1);
        __ASSERT(zmk_analytics_save() == -EAGAIN, "save rate limit");
        k_sleep(K_MSEC(10010));
        fail_write = true;
        __ASSERT(zmk_analytics_save() == -EIO, "injected save failure");
        zmk_analytics_capture(&stats, &bigrams, &status);
        __ASSERT(status.generation == 1 && status.dirty && status.error_count_session == 1,
                 "failed save changed generation or lost dirty state");
        fail_write = false;
        k_sleep(K_MSEC(10010));
        __ASSERT(zmk_analytics_save() == 0, "save generation 2 after failure");
        zmk_analytics_capture(&stats, &bigrams, &status);
        __ASSERT(status.generation == 2 && !status.dirty, "generation 2 not committed");
    } else if (strcmp(step, "verify") == 0 || strcmp(step, "recover") == 0) {
        __ASSERT(status.generation == 2 && stats.presses[0][0] == 2 && stats.presses[1][0] == 2,
                 "saved counts + early startup event not restored");
        __ASSERT(bigrams.counts[0][1] == 1 && bigrams.counts[1][1] == 1,
                 "directed bigrams not restored");
        if (strcmp(step, "recover") == 0) {
            __ASSERT(status.recovered, "corrupt slot not reported");
        }
        __ASSERT(settings_load_subtree("analytics_poc") == 0, "reload");
        zmk_analytics_capture(&stats, &bigrams, &status);
        __ASSERT(stats.presses[0][0] == 2, "reload merged saved counts twice");
        __ASSERT(bigrams.counts[0][1] == 1 && bigrams.counts[1][1] == 1,
                 "reload merged saved bigrams twice");
    } else if (strcmp(step, "corrupt") == 0 || strcmp(step, "corrupt-both") == 0) {
        /* 'a' contains generation 1. Replace it with a truncated record,
         * keeping generation 2 in 'b' recoverable across a real NVS reload. */
        const uint8_t bad[] = {1, 2, 3};
        __ASSERT(settings_save_one("analytics_poc/a", bad, sizeof(bad)) == 0, "corrupt a");
        if (strcmp(step, "corrupt-both") == 0) {
            __ASSERT(settings_save_one("analytics_poc/b", bad, sizeof(bad)) == 0, "corrupt b");
        }
    } else if (strcmp(step, "blocked") == 0 || strcmp(step, "layout") == 0) {
        __ASSERT(status.generation == 0 && stats.presses[0][0] == 1, "invalid data restored");
        __ASSERT(strcmp(status.state, strcmp(step, "layout") == 0 ? "incompatible_layout" : "error") == 0,
                 "bad state");
        __ASSERT(zmk_analytics_save() == -EACCES, "bad/incompatible slots overwritten");
    } else {
        __ASSERT(false, "unknown step");
    }
    bool marker_found = false;
    settings_load_subtree_direct("unrelated", check_marker, &marker_found);
    __ASSERT(marker_found, "unrelated settings lost");
    printf("PASS: storage %s\n", step);
    exit(0);
}

K_WORK_DELAYABLE_DEFINE(probe_work, probe);

static int start_probe(void) {
    /* Before main's settings_load: merge must preserve this input. */
    press(0);
    k_work_schedule(&probe_work, K_MSEC(100));
    return 0;
}

SYS_INIT(start_probe, APPLICATION, 99);
