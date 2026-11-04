/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <analytics/collector.h>
#include <analytics/persistence.h>

#define ANALYTICS_UART DT_NODELABEL(analytics_cdc_acm_uart)

BUILD_ASSERT(DT_NODE_HAS_STATUS(ANALYTICS_UART, okay), "Include usb-poc.overlay");
#if DT_HAS_CHOSEN(zmk_studio_rpc_uart)
BUILD_ASSERT(!DT_SAME_NODE(ANALYTICS_UART, DT_CHOSEN(zmk_studio_rpc_uart)),
             "Analytics must not share the Studio UART");
#endif

static const struct device *const serial = DEVICE_DT_GET(ANALYTICS_UART);

/* Only the diagnostic thread performs UART I/O; never an input callback.
 * The snapshot is copied once and serialized after releasing the spinlock. */
static bool connected(void) {
    uint32_t dtr = 0;
    return uart_line_ctrl_get(serial, UART_LINE_CTRL_DTR, &dtr) == 0 && dtr;
}

static bool send_text(const char *text) {
    size_t remaining = strlen(text);
    int64_t deadline = k_uptime_get() + 2000;
    while (remaining) {
        if (!connected() || k_uptime_get() >= deadline) {
            return false;
        }
        /* v0.3's CDC poll_out discards old bytes when its TX ring fills.
         * CDC fifo_fill instead returns the accepted length. There is one
         * producer on this private UART; its driver schedules USB TX work. */
        int written = uart_fifo_fill(serial, (const uint8_t *)text, remaining);
        if (written < 0) {
            return false;
        }
        text += written;
        remaining -= written;
        if (remaining) {
            k_sleep(K_MSEC(2));
        }
    }
    return true;
}

static void send_snapshot(void) {
    struct analytics_poc_snapshot stats;
    struct analytics_storage_status storage;
    char buffer[224];
    zmk_analytics_capture(&stats, &storage);
    if (!send_text("{\"protocol\":\"zmk-analytics-poc\",\"version\":2,"
                   "\"development_unprotected\":true,\"persistent\":")) {
        return;
    }
    snprintf(buffer, sizeof(buffer), "%s,\"storage\":{\"state\":\"%s\",\"generation\":%lu,"
             "\"error_count_session\":%lu,\"last_error\":%d,\"dirty\":%s,\"recovered\":%s},",
             storage.enabled ? "true" : "false", storage.state, (unsigned long)storage.generation,
             (unsigned long)storage.error_count_session, storage.last_error,
             storage.dirty ? "true" : "false", storage.recovered ? "true" : "false");
    if (!send_text(buffer)) { return; }
#if IS_ENABLED(CONFIG_ZMK_ANALYTICS_PERSISTENCE)
    if (!send_text("\"physical_layout_id\":\"" CONFIG_ZMK_ANALYTICS_LAYOUT_ID "\","
                   "\"layout_tracking\":\"manual\",")) { return; }
#endif
    if (!send_text("\"measurement\":\"source_events\",\"presses\":[")) {
        return;
    }
    for (unsigned int i = 0; i < ANALYTICS_POSITION_COUNT; ++i) {
        snprintf(buffer, sizeof(buffer), "%s[%lu,%lu]", i ? "," : "",
                 (unsigned long)stats.presses[i][ANALYTICS_LOCAL],
                 (unsigned long)stats.presses[i][ANALYTICS_PERIPHERAL]);
        if (!send_text(buffer)) {
            return;
        }
    }
    snprintf(buffer, sizeof(buffer), "],\"releases\":[%lu,%lu],\"invalid_positions\":%lu,"
             "\"saturated\":%s}\n", (unsigned long)stats.releases[ANALYTICS_LOCAL],
             (unsigned long)stats.releases[ANALYTICS_PERIPHERAL],
             (unsigned long)stats.invalid_positions, stats.saturated ? "true" : "false");
    send_text(buffer);
}

static void diagnostic_thread(void *a, void *b, void *c) {
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);
    if (!device_is_ready(serial)) {
        return;
    }
    char command[8];
    size_t used = 0;
    bool overflow = false;
    while (true) {
        unsigned char byte;
        if (!connected()) {
            used = 0;
            overflow = false;
            /* Discard stale commands, not statistics, when the host closes. */
            while (uart_poll_in(serial, &byte) == 0) {}
            k_sleep(K_MSEC(100));
            continue;
        }
        if (uart_poll_in(serial, &byte) != 0) {
            k_sleep(K_MSEC(20));
            continue;
        }
        if (byte == '\n') {
            command[used] = '\0';
            if (!overflow && strcmp(command, "GET") == 0) {
                send_snapshot();
            } else if (!overflow && strcmp(command, "SAVE") == 0) {
                int error = zmk_analytics_save();
                char reply[64];
                snprintf(reply, sizeof(reply), "{\"saved\":%s,\"error\":%d}\n",
                         error == 0 ? "true" : "false", error);
                send_text(reply);
            } else {
                send_text("{\"error\":\"unknown_command\"}\n");
            }
            used = 0;
            overflow = false;
            /* Bound request rate even if a host continuously sends GET. */
            k_sleep(K_MSEC(100));
        } else if (byte != '\r') {
            if (used < sizeof(command) - 1) {
                command[used++] = (char)byte;
            } else {
                overflow = true;
            }
        }
    }
}

K_THREAD_DEFINE(analytics_usb_thread, 3072, diagnostic_thread, NULL, NULL, NULL, 14, 0, 0);
