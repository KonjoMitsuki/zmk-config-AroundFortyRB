/* SPDX-License-Identifier: MIT */
#include <zephyr/kernel.h>
#include <zmk/events/position_state_changed.h>
#include <analytics/collector.h>

static struct analytics_poc_snapshot counters;
static struct k_spinlock counters_lock;

int __real_raise_zmk_position_state_changed(struct zmk_position_state_changed event);

/* Intercept construction of NEW events, not zmk_event_manager_raise(). Combo
 * RELEASE/RAISE and hold-tap RAISE_AT operate on existing event envelopes and
 * bypass this symbol. No listener ordering or timestamp-based deduplication.
 * Do not hold the spinlock across downstream callbacks (which can re-enter). */
int __wrap_raise_zmk_position_state_changed(struct zmk_position_state_changed event) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    analytics_poc_record(&counters, event.position, event.source, event.state);
    k_spin_unlock(&counters_lock, key);
    return __real_raise_zmk_position_state_changed(event);
}

void zmk_analytics_poc_copy(struct analytics_poc_snapshot *destination) {
    k_spinlock_key_t key = k_spin_lock(&counters_lock);
    *destination = counters;
    k_spin_unlock(&counters_lock, key);
}
