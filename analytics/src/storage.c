/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <analytics/persistence.h>

#if IS_ENABLED(CONFIG_ZMK_ANALYTICS_PERSISTENCE)
#include "analytics_layout_hash.h"

BUILD_ASSERT(ANALYTICS_RECORD_SIZE <= 4000, "Snapshot must fit the audited 4KB NVS sector");
BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(zmk_physical_layout) <= 1,
             "Persistence currently requires a single physical layout");

static K_MUTEX_DEFINE(storage_lock);
static K_SEM_DEFINE(storage_ready, 0, 1);
static struct analytics_saved_record slots[2];
static enum analytics_slot_state slot_states[2];
static struct analytics_storage_status status = {.state = "loading", .enabled = true};
static atomic_t ready;
static bool writable;
static int current = -1;
static int64_t last_attempt = -10000;

static void note_error(int error) {
    status.state = "error";
    status.last_error = error;
    if (status.error_count_session != UINT32_MAX) { ++status.error_count_session; }
}

static int load_slot(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    if (!name) { return 0; }
    int slot = strcmp(name, "a") == 0 ? 0 : strcmp(name, "b") == 0 ? 1 : -1;
    if (slot < 0 || atomic_get(&ready)) { return 0; }
    uint8_t bytes[ANALYTICS_RECORD_SIZE];
    slot_states[slot] = ANALYTICS_SLOT_BAD;
    if (len == sizeof(bytes) && read_cb(cb_arg, bytes, sizeof(bytes)) == (ssize_t)sizeof(bytes) &&
        analytics_decode_record(&slots[slot], bytes, len) == 0) {
        slot_states[slot] = ANALYTICS_SLOT_VALID;
    }
    return 0; /* Report locally; do not break unrelated Bluetooth/Studio loading. */
}

static int restore(void) {
    /* settings invokes commit under settings_lock. Later subtree/full reloads
     * must not acquire storage_lock while a saver holds it and awaits settings. */
    if (atomic_get(&ready)) { return 0; }
    k_mutex_lock(&storage_lock, K_FOREVER);
    if (!atomic_get(&ready)) {
        int best = analytics_choose_record(slots, slot_states, analytics_layout_hash);
        if (best >= 0) {
            current = best;
            zmk_analytics_poc_restore(&slots[best].stats);
            status.generation = slots[best].generation;
            status.state = "ok";
            status.recovered = slot_states[1 - best] == ANALYTICS_SLOT_BAD;
            if (status.recovered) { status.last_error = -EBADMSG; }
            writable = true;
        } else if (best == -ENOENT) {
            status.state = "no_valid_snapshot";
            writable = true;
        } else {
            note_error(best);
            /* Preserve corrupt or incompatible data for diagnosis; no auto-overwrite. */
            if (best == -EXDEV) { status.state = "incompatible_layout"; }
        }
        atomic_set(&ready, 1);
        k_sem_give(&storage_ready);
    }
    k_mutex_unlock(&storage_lock);
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(analytics_poc, "analytics_poc", NULL, load_slot, restore, NULL);

void zmk_analytics_capture(struct analytics_poc_snapshot *stats, struct analytics_storage_status *out) {
    k_mutex_lock(&storage_lock, K_FOREVER);
    zmk_analytics_poc_copy(stats);
    *out = status;
    out->dirty = current < 0 || !analytics_stats_equal(stats, &slots[current].stats);
    k_mutex_unlock(&storage_lock);
}

int zmk_analytics_save(void) {
    k_mutex_lock(&storage_lock, K_FOREVER);
    int result = 0;
    if (!atomic_get(&ready) || !writable) {
        result = atomic_get(&ready) ? -EACCES : -EAGAIN;
        goto done;
    }
    struct analytics_saved_record next = {0};
    zmk_analytics_poc_copy(&next.stats);
    if (current >= 0 && analytics_stats_equal(&next.stats, &slots[current].stats)) { goto done; }
    if (k_uptime_get() - last_attempt < 10000) {
        result = -EAGAIN; /* Explicit SAVE is rate-limited, even if a host loops requests. */
        goto done;
    }
    last_attempt = k_uptime_get();
    if (status.generation == UINT32_MAX) {
        result = -EOVERFLOW;
        note_error(result);
        goto done;
    }
    next.generation = status.generation + 1;
    memcpy(next.layout_hash, analytics_layout_hash, 32);
    uint8_t bytes[ANALYTICS_RECORD_SIZE];
    analytics_encode_record(bytes, &next);
    int target = current < 0 ? 0 : 1 - current;
    result = settings_save_one(target == 0 ? "analytics_poc/a" : "analytics_poc/b", bytes, sizeof(bytes));
    if (result) {
        note_error(result);
    } else {
        slots[target] = next;
        slot_states[target] = ANALYTICS_SLOT_VALID;
        current = target;
        status.generation = next.generation;
        status.last_error = 0;
        status.state = "ok";
    }
done:
    k_mutex_unlock(&storage_lock);
    return result;
}

static void saver(void *a, void *b, void *c) {
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    k_sem_take(&storage_ready, K_FOREVER);
    while (true) {
        k_sleep(K_SECONDS(CONFIG_ZMK_ANALYTICS_SAVE_INTERVAL_SECONDS));
        zmk_analytics_save();
    }
}

K_THREAD_DEFINE(analytics_saver, 3072, saver, NULL, NULL, NULL, 14, 0, 0);

#else

void zmk_analytics_capture(struct analytics_poc_snapshot *stats, struct analytics_storage_status *out) {
    zmk_analytics_poc_copy(stats);
    *out = (struct analytics_storage_status){.state = "disabled"};
}

int zmk_analytics_save(void) { return -ENOTSUP; }

#endif
