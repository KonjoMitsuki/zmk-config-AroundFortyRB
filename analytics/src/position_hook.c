/* SPDX-License-Identifier: MIT */
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/events/position_state_changed.h>
#include <analytics/bigram.h>
#include <analytics/collector.h>
#include <analytics/recent.h>
#include <analytics/timing.h>

static struct analytics_poc_snapshot counters;
static struct analytics_bigram_counts bigrams;
static struct analytics_timing_state timing;
static struct analytics_recent_log recent;
static struct k_spinlock counters_lock;
static bool initialized;

int __real_raise_zmk_position_state_changed(struct zmk_position_state_changed event);

static void ensure_initialized_locked(void) {
    if (initialized) {
        return;
    }
    analytics_poc_init(&counters, CONFIG_ZMK_ANALYTICS_PAIR_LIST);
    analytics_bigrams_init(&bigrams);
    analytics_timing_init(&timing);
    analytics_recent_init(&recent);
    initialized = true;
}

/* Intercept construction of NEW events, not zmk_event_manager_raise(). Combo
 * RELEASE/RAISE and hold-tap RAISE_AT operate on existing event envelopes and
 * bypass this symbol. No listener ordering or timestamp-based deduplication.
 * Do not hold the spinlock across downstream callbacks (which can re-enter). */
int __wrap_raise_zmk_position_state_changed(struct zmk_position_state_changed event) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    analytics_poc_record(&counters, event.position, event.source, event.state);
    struct analytics_timing_result result =
        analytics_timing_record(&counters, &timing, event.position, event.source,
                                event.state, event.timestamp);
    if (result.new_press) {
        if (result.previous_valid) {
            analytics_bigram_record(&bigrams, result.previous_position, event.position);
        }
        analytics_recent_record(&recent, event.position, event.source, event.timestamp);
    }
    k_spin_unlock(&counters_lock, key);
    return __real_raise_zmk_position_state_changed(event);
}

void zmk_analytics_snapshot_copy(struct analytics_poc_snapshot *stats,
                                 struct analytics_bigram_counts *bigrams_out) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    *stats = counters;
    *bigrams_out = bigrams;
    k_spin_unlock(&counters_lock, key);
}

void zmk_analytics_poc_copy(struct analytics_poc_snapshot *destination) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    *destination = counters;
    k_spin_unlock(&counters_lock, key);
}

void zmk_analytics_bigrams_copy(struct analytics_bigram_counts *destination) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    *destination = bigrams;
    k_spin_unlock(&counters_lock, key);
}

void zmk_analytics_recent_copy(struct analytics_recent_log *destination) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    *destination = recent;
    k_spin_unlock(&counters_lock, key);
}

void zmk_analytics_poc_restore(struct analytics_poc_snapshot *saved) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    /* Input may already have arrived before settings_load(): retain it. */
    analytics_poc_merge(&counters, saved);
    k_spin_unlock(&counters_lock, key);
}

void zmk_analytics_bigrams_restore(const struct analytics_bigram_counts *saved) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    analytics_bigrams_merge(&bigrams, saved);
    k_spin_unlock(&counters_lock, key);
}

static int analytics_init(void) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    ensure_initialized_locked();
    k_spin_unlock(&counters_lock, key);
    return 0;
}

SYS_INIT(analytics_init, APPLICATION, 0);
