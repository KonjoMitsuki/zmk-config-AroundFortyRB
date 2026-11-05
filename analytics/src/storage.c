/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <analytics/persistence.h>

#if IS_ENABLED(CONFIG_ZMK_ANALYTICS_PERSISTENCE)
#include "analytics_layout_hash.h"

BUILD_ASSERT(ANALYTICS_RECORD_SIZE <= 4000, "Snapshot must fit the audited 4KB NVS sector");
BUILD_ASSERT(ANALYTICS_BIGRAM_RECORD_SIZE <= 4000, "Bigram record must fit one NVS sector");
BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(zmk_physical_layout) <= 1,
             "Persistence currently requires a single physical layout");

#define ANALYTICS_MAX_RECORD_SIZE ANALYTICS_RECORD_SIZE

static K_MUTEX_DEFINE(storage_lock);
static K_SEM_DEFINE(storage_ready, 0, 1);
static struct analytics_saved_record slots[2];
static enum analytics_slot_state slot_states[2];
static uint32_t slot_versions[2];
static struct analytics_saved_bigrams bigram_slots[2];
static enum analytics_slot_state bigram_states[2];
static struct analytics_storage_status status = {.state = "loading", .enabled = true};
static atomic_t ready;
static bool writable;
static int current = -1;
static int current_bigram = -1;
static int64_t last_attempt = -10000;

/* settings callbacks and the saver are serialized by settings_lock or
 * storage_lock; these buffers keep the large records off small thread stacks. */
static uint8_t load_bytes[ANALYTICS_MAX_RECORD_SIZE];
static struct analytics_saved_record next_main;
static struct analytics_saved_bigrams next_bigrams;
static uint8_t main_bytes[ANALYTICS_RECORD_SIZE];
static uint8_t bigram_bytes[ANALYTICS_BIGRAM_RECORD_SIZE];

static void note_error(int error) {
    status.state = "error";
    status.last_error = error;
    if (status.error_count_session != UINT32_MAX) {
        ++status.error_count_session;
    }
}

static int load_slot(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    if (!name || atomic_get(&ready)) {
        return 0;
    }
    int main_slot = strcmp(name, "a") == 0 ? 0 : strcmp(name, "b") == 0 ? 1 : -1;
    int bigram_slot = strcmp(name, "c") == 0 ? 0 : strcmp(name, "d") == 0 ? 1 : -1;
    if (main_slot < 0 && bigram_slot < 0) {
        return 0;
    }
    if (main_slot >= 0) {
        slot_states[main_slot] = ANALYTICS_SLOT_BAD;
        if ((len == ANALYTICS_RECORD_SIZE || len == ANALYTICS_RECORD_SIZE_V1) &&
            read_cb(cb_arg, load_bytes, len) == (ssize_t)len &&
            analytics_decode_record(&slots[main_slot], load_bytes, len) == 0) {
            slot_states[main_slot] = ANALYTICS_SLOT_VALID;
            slot_versions[main_slot] = slots[main_slot].schema_version;
        }
    } else {
        bigram_states[bigram_slot] = ANALYTICS_SLOT_BAD;
        if (len == ANALYTICS_BIGRAM_RECORD_SIZE &&
            read_cb(cb_arg, load_bytes, len) == (ssize_t)len &&
            analytics_decode_bigram_record(&bigram_slots[bigram_slot], load_bytes, len) == 0) {
            bigram_states[bigram_slot] = ANALYTICS_SLOT_VALID;
        }
    }
    return 0; /* Report locally; do not break unrelated Bluetooth/Studio loading. */
}

static int restore(void) {
    /* settings invokes commit under settings_lock. Later subtree/full reloads
     * must not acquire storage_lock while a saver holds it and awaits settings. */
    if (atomic_get(&ready)) {
        return 0;
    }
    k_mutex_lock(&storage_lock, K_FOREVER);
    if (!atomic_get(&ready)) {
        int best = analytics_choose_record(slots, slot_states, analytics_layout_hash);
        if (best >= 0) {
            current = best;
            zmk_analytics_poc_restore(&slots[best].stats);
            status.generation = slots[best].generation;
            status.state = "ok";
            status.recovered = slot_states[1 - best] == ANALYTICS_SLOT_BAD;
            if (status.recovered) {
                status.last_error = -EBADMSG;
            }
            writable = true;

            int best_bigram =
                analytics_choose_bigram_record(bigram_slots, bigram_states, analytics_layout_hash);
            if (best_bigram >= 0) {
                current_bigram = best_bigram;
                zmk_analytics_bigrams_restore(&bigram_slots[best_bigram].counts);
            } else {
                /* Bigrams are auxiliary: a missing or corrupt matrix must not
                 * block the main counter record or prevent a fresh save. */
                current_bigram = -1;
            }
        } else if (best == -ENOENT) {
            status.state = "no_valid_snapshot";
            writable = true;
        } else {
            note_error(best);
            /* Preserve corrupt or incompatible data for diagnosis; no auto-overwrite. */
            if (best == -EXDEV) {
                status.state = "incompatible_layout";
            }
        }
        atomic_set(&ready, 1);
        k_sem_give(&storage_ready);
    }
    k_mutex_unlock(&storage_lock);
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(analytics_poc, "analytics_poc", NULL, load_slot, restore, NULL);

void zmk_analytics_capture(struct analytics_poc_snapshot *stats,
                           struct analytics_bigram_counts *bigrams,
                           struct analytics_storage_status *out) {
    k_mutex_lock(&storage_lock, K_FOREVER);
    zmk_analytics_snapshot_copy(stats, bigrams);
    *out = status;
    out->dirty = current < 0 || slot_versions[current] == 1 ||
                 !analytics_stats_equal(stats, &slots[current].stats) ||
                 current_bigram < 0 ||
                 !analytics_bigrams_equal(bigrams, &bigram_slots[current_bigram].counts);
    k_mutex_unlock(&storage_lock);
}

int zmk_analytics_save(void) {
    k_mutex_lock(&storage_lock, K_FOREVER);
    int result = 0;
    if (!atomic_get(&ready) || !writable) {
        result = atomic_get(&ready) ? -EACCES : -EAGAIN;
        goto done;
    }
    zmk_analytics_snapshot_copy(&next_main.stats, &next_bigrams.counts);
    bool main_dirty = current < 0 || slot_versions[current] == 1 ||
                      !analytics_stats_equal(&next_main.stats, &slots[current].stats);
    bool bigram_dirty =
        current_bigram < 0 ||
        !analytics_bigrams_equal(&next_bigrams.counts, &bigram_slots[current_bigram].counts);
    if (!main_dirty && !bigram_dirty) {
        goto done;
    }
    if (k_uptime_get() - last_attempt < 10000) {
        result = -EAGAIN; /* Explicit SAVE is rate-limited, even if a host loops requests. */
        goto done;
    }
    last_attempt = k_uptime_get();

    if (main_dirty) {
        if (status.generation == UINT32_MAX) {
            result = -EOVERFLOW;
            note_error(result);
            goto done;
        }
        next_main.generation = status.generation + 1;
        next_main.schema_version = 2;
        memcpy(next_main.layout_hash, analytics_layout_hash, ANALYTICS_LAYOUT_HASH_SIZE);
        analytics_encode_record(main_bytes, &next_main);
        int target = current < 0 ? 0 : 1 - current;
        result = settings_save_one(target == 0 ? "analytics_poc/a" : "analytics_poc/b",
                                   main_bytes, sizeof(main_bytes));
        if (result) {
            note_error(result);
            goto done;
        }
        slots[target] = next_main;
        slot_states[target] = ANALYTICS_SLOT_VALID;
        slot_versions[target] = 2;
        current = target;
        status.generation = next_main.generation;
    }

    if (bigram_dirty) {
        uint32_t base = current_bigram >= 0 ? bigram_slots[current_bigram].generation
                                            : status.generation;
        if (base == UINT32_MAX) {
            result = -EOVERFLOW;
            note_error(result);
            goto done;
        }
        next_bigrams.generation = base + 1;
        next_bigrams.schema_version = 1;
        memcpy(next_bigrams.layout_hash, analytics_layout_hash, ANALYTICS_LAYOUT_HASH_SIZE);
        analytics_encode_bigram_record(bigram_bytes, &next_bigrams);
        int target = current_bigram < 0 ? 0 : 1 - current_bigram;
        result = settings_save_one(target == 0 ? "analytics_poc/c" : "analytics_poc/d",
                                   bigram_bytes, sizeof(bigram_bytes));
        if (result) {
            note_error(result);
            goto done;
        }
        bigram_slots[target] = next_bigrams;
        bigram_states[target] = ANALYTICS_SLOT_VALID;
        current_bigram = target;
    }

    status.last_error = 0;
    status.state = "ok";
done:
    k_mutex_unlock(&storage_lock);
    return result;
}

static void saver(void *a, void *b, void *c) {
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);
    k_sem_take(&storage_ready, K_FOREVER);
    while (true) {
        k_sleep(K_SECONDS(CONFIG_ZMK_ANALYTICS_SAVE_INTERVAL_SECONDS));
        zmk_analytics_save();
    }
}

K_THREAD_DEFINE(analytics_saver, 3072, saver, NULL, NULL, NULL, 14, 0, 0);

#else

void zmk_analytics_capture(struct analytics_poc_snapshot *stats,
                           struct analytics_bigram_counts *bigrams,
                           struct analytics_storage_status *out) {
    zmk_analytics_snapshot_copy(stats, bigrams);
    *out = (struct analytics_storage_status){.state = "disabled"};
}

int zmk_analytics_save(void) { return -ENOTSUP; }

#endif
