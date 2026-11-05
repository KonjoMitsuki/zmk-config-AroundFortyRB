/* SPDX-License-Identifier: MIT */

#define DT_DRV_COMPAT zmk_behavior_aroundforty_status_led

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/split/central.h>
#include <zmk/split/transport/central.h>
#else
#include <zmk/events/split_peripheral_status_changed.h>
#endif

enum status_color {
    STATUS_OFF = 0,
    STATUS_AML_ON = 2,     /* green: automatic AML enabled */
    STATUS_CLIP_ON = 5,    /* magenta: Clip Studio enabled */
    STATUS_AML_ACTIVE = 6, /* cyan: mouse layer currently active */
};

/*
 * Adapter for zmk-rgbled-widget 8756cb7. Its persistent color and message
 * queue are exported symbols, but not a public API. CMake checks widget.c's
 * hash before allowing this adapter to compile. Only the widget's own LED
 * thread touches the GPIOs, so battery/connection blinks still finish normally
 * and then restore the current status color.
 */
extern uint8_t led_layer_color;
extern struct k_msgq led_msgq;
struct widget_blink_item {
    uint8_t color;
    uint16_t duration_ms;
    uint16_t sleep_ms;
};

static int last_queued_color = -1;

/* Called only from the system workqueue, on either half. */
static void set_status_color(uint8_t color) {
    led_layer_color = color;
    if (last_queued_color == color) {
        return;
    }

    const struct widget_blink_item item = {.color = color};
    if (k_msgq_put(&led_msgq, &item, K_NO_WAIT) == 0) {
        last_queued_color = color;
    }
    /* Queue full: the next refresh retries. The blink thread also restores
     * led_layer_color after any battery/connection indication in progress. */
}

static void update_status(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(status_work, update_status);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

static void sync_clip_to_left(bool clip_on) {
    struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_DRV_INST(0)),
        .param1 = clip_on,
    };
    const struct zmk_behavior_binding_event event = {.timestamp = k_uptime_get()};

    STRUCT_SECTION_FOREACH(zmk_split_transport_central, transport) {
        if (!transport->api->get_available_source_ids) {
            continue;
        }
        uint8_t sources[ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT];
        int count = transport->api->get_available_source_ids(sources);
        for (int i = 0; i < count; i++) {
            zmk_split_central_invoke_behavior(sources[i], &binding, event, true);
        }
    }
}

static void update_status(struct k_work *work) {
    /* Read each actual layer bit rather than the highest layer, so Settings,
     * AML-On and Clip Studio never hide each other's indicators. */
    const bool aml_active = zmk_keymap_layer_active(DT_INST_PROP(0, aml_layer));
    const bool aml_on = zmk_keymap_layer_active(DT_INST_PROP(0, aml_on_layer));
    set_status_color(aml_active ? STATUS_AML_ACTIVE : aml_on ? STATUS_AML_ON : STATUS_OFF);
    sync_clip_to_left(zmk_keymap_layer_active(DT_INST_PROP(0, clip_layer)));

    /* A successful BLE enqueue does not acknowledge delivery. Re-send the
     * absolute state every two seconds to recover from reconnects, missed
     * packets and a left-half reboot without toggling the layer again. */
    k_work_schedule(&status_work, K_SECONDS(2));
}

static int status_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *event = as_zmk_layer_state_changed(eh);
    if (event->layer == DT_INST_PROP(0, clip_layer) ||
        event->layer == DT_INST_PROP(0, aml_layer) ||
        event->layer == DT_INST_PROP(0, aml_on_layer)) {
        k_work_reschedule(&status_work, K_NO_WAIT);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(aroundforty_status, status_listener);
ZMK_SUBSCRIPTION(aroundforty_status, zmk_layer_state_changed);

#else

static atomic_t clip_enabled;

static void update_status(struct k_work *work) {
    set_status_color(atomic_get(&clip_enabled) ? STATUS_CLIP_ON : STATUS_OFF);
    k_work_schedule(&status_work, K_SECONDS(2));
}

static int status_listener(const zmk_event_t *eh) {
    const struct zmk_split_peripheral_status_changed *event =
        as_zmk_split_peripheral_status_changed(eh);
    if (!event->connected) {
        /* Do not leave a stale ON indication when the right half is absent. */
        atomic_set(&clip_enabled, 0);
        k_work_reschedule(&status_work, K_NO_WAIT);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(aroundforty_status, status_listener);
ZMK_SUBSCRIPTION(aroundforty_status, zmk_split_peripheral_status_changed);

#endif

static int status_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
#if !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (binding->param1 > 1) {
        return -EINVAL;
    }
    /* Absolute state, not a toggle: duplicate messages are harmless. */
    atomic_set(&clip_enabled, binding->param1);
    k_work_reschedule(&status_work, K_NO_WAIT);
#endif
    return ZMK_BEHAVIOR_OPAQUE;
}

static int status_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api status_api = {
    .binding_pressed = status_pressed,
    .binding_released = status_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &status_api);

static int status_init(void) {
    k_work_schedule(&status_work, K_MSEC(200));
    return 0;
}

SYS_INIT(status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
