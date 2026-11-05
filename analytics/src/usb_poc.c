/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <analytics/bigram.h>
#include <analytics/collector.h>
#include <analytics/persistence.h>
#include <analytics/recent.h>

#define ANALYTICS_UART DT_NODELABEL(analytics_cdc_acm_uart)

BUILD_ASSERT(DT_NODE_HAS_STATUS(ANALYTICS_UART, okay), "Include usb-poc.overlay");
#if DT_HAS_CHOSEN(zmk_studio_rpc_uart)
BUILD_ASSERT(!DT_SAME_NODE(ANALYTICS_UART, DT_CHOSEN(zmk_studio_rpc_uart)),
             "Analytics must not share the Studio UART");
#endif

static const struct device *const serial = DEVICE_DT_GET(ANALYTICS_UART);

/* Large snapshots live in BSS, never on the 3 KB USB thread stack. */
static struct analytics_poc_snapshot usb_stats;
static struct analytics_bigram_counts usb_bigrams;
static struct analytics_recent_log usb_recent;

static bool connected(void) {
    uint32_t dtr = 0;
    return uart_line_ctrl_get(serial, UART_LINE_CTRL_DTR, &dtr) == 0 && dtr;
}

static bool send_text(const char *text) {
    size_t remaining = strlen(text);
    int64_t deadline = k_uptime_get() + 5000;
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

static bool send_values8(const uint16_t values[ANALYTICS_TIME_BINS]) {
    char buffer[96];
    snprintf(buffer, sizeof(buffer), "[%u,%u,%u,%u,%u,%u,%u,%u]",
             (unsigned int)values[0], (unsigned int)values[1], (unsigned int)values[2],
             (unsigned int)values[3], (unsigned int)values[4], (unsigned int)values[5],
             (unsigned int)values[6], (unsigned int)values[7]);
    return send_text(buffer);
}

static bool send_values6(const uint16_t values[ANALYTICS_PAIR_BINS]) {
    char buffer[72];
    snprintf(buffer, sizeof(buffer), "[%u,%u,%u,%u,%u,%u]",
             (unsigned int)values[0], (unsigned int)values[1], (unsigned int)values[2],
             (unsigned int)values[3], (unsigned int)values[4], (unsigned int)values[5]);
    return send_text(buffer);
}

static bool send_position_histogram(const uint16_t values[][ANALYTICS_TIME_BINS]) {
    if (!send_text("[")) {
        return false;
    }
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        if (p && !send_text(",")) {
            return false;
        }
        if (!send_values8(values[p])) {
            return false;
        }
    }
    return send_text("]");
}

static void send_snapshot(void) {
    struct analytics_storage_status storage;
    char buffer[384];
    zmk_analytics_capture(&usb_stats, &usb_bigrams, &storage);
    if (!send_text("{\"protocol\":\"zmk-analytics-poc\",\"version\":3,"
                   "\"development_unprotected\":true,\"persistent\":")) {
        return;
    }
    snprintf(buffer, sizeof(buffer), "%s,\"storage\":{\"state\":\"%s\",\"generation\":%lu,"
             "\"error_count_session\":%lu,\"last_error\":%d,\"dirty\":%s,\"recovered\":%s},",
             storage.enabled ? "true" : "false", storage.state,
             (unsigned long)storage.generation,
             (unsigned long)storage.error_count_session, storage.last_error,
             storage.dirty ? "true" : "false", storage.recovered ? "true" : "false");
    if (!send_text(buffer)) {
        return;
    }
#if IS_ENABLED(CONFIG_ZMK_ANALYTICS_PERSISTENCE)
    if (!send_text("\"physical_layout_id\":\"" CONFIG_ZMK_ANALYTICS_LAYOUT_ID "\","
                   "\"layout_tracking\":\"manual\",")) {
        return;
    }
#endif
    if (!send_text("\"measurement\":\"source_events\",\"presses\":[")) {
        return;
    }
    for (unsigned int i = 0; i < ANALYTICS_POSITION_COUNT; ++i) {
        snprintf(buffer, sizeof(buffer), "%s[%lu,%lu]", i ? "," : "",
                 (unsigned long)usb_stats.presses[i][ANALYTICS_LOCAL],
                 (unsigned long)usb_stats.presses[i][ANALYTICS_PERIPHERAL]);
        if (!send_text(buffer)) {
            return;
        }
    }
    snprintf(buffer, sizeof(buffer), "],\"releases\":[%lu,%lu],\"invalid_positions\":%lu,"
             "\"saturated\":%s,\"quality\":{\"duplicate_downs\":%lu,\"orphan_releases\":%lu,"
             "\"timing_discards\":%lu,\"pair_config_errors\":%lu,"
             "\"histogram_saturated\":%s},",
             (unsigned long)usb_stats.releases[ANALYTICS_LOCAL],
             (unsigned long)usb_stats.releases[ANALYTICS_PERIPHERAL],
             (unsigned long)usb_stats.invalid_positions,
             usb_stats.saturated ? "true" : "false",
             (unsigned long)usb_stats.duplicate_downs,
             (unsigned long)usb_stats.orphan_releases,
             (unsigned long)usb_stats.timing_discards,
             (unsigned long)usb_stats.pair_config_errors,
             usb_stats.histogram_saturated ? "true" : "false");
    if (!send_text(buffer)) {
        return;
    }
    if (!send_text("\"timing\":{\"hold_bins_ms\":[20,40,60,80,100,200,500],"
                   "\"interval_bins_ms\":[50,100,150,200,300,500,1000],"
                   "\"pair_bins_ms\":[20,40,60,80,100],\"hold\":")) {
        return;
    }
    if (!send_position_histogram(usb_stats.hold)) {
        return;
    }
    if (!send_text(",\"same_key\":")) {
        return;
    }
    if (!send_position_histogram(usb_stats.same_key)) {
        return;
    }
    if (!send_text(",\"idle_before_press\":")) {
        return;
    }
    if (!send_position_histogram(usb_stats.idle_before_press)) {
        return;
    }
    if (!send_text(",\"pairs\":[")) {
        return;
    }
    for (unsigned int i = 0; i < usb_stats.pair_count; ++i) {
        snprintf(buffer, sizeof(buffer),
                 "%s{\"positions\":[%u,%u],\"near\":[",
                 i ? "," : "", (unsigned int)usb_stats.pairs[i][0],
                 (unsigned int)usb_stats.pairs[i][1]);
        if (!send_text(buffer)) {
            return;
        }
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            if (c && !send_text(",")) {
                return;
            }
            if (!send_values6(usb_stats.pair_near[i][c])) {
                return;
            }
        }
        if (!send_text("],\"overlap\":[")) {
            return;
        }
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            if (c && !send_text(",")) {
                return;
            }
            if (!send_values6(usb_stats.pair_overlap[i][c])) {
                return;
            }
        }
        if (!send_text("]}")) {
            return;
        }
    }
    send_text("]}}\n");
}

static void send_bigrams(void) {
    zmk_analytics_bigrams_copy(&usb_bigrams);
    char buffer[384];
    if (!send_text("{\"protocol\":\"zmk-analytics-poc\",\"version\":3,\"kind\":\"bigrams\","
                   "\"positions\":")) {
        return;
    }
    snprintf(buffer, sizeof(buffer), "%u,\"saturated\":%s,\"counts\":[",
             (unsigned int)ANALYTICS_POSITION_COUNT,
             usb_bigrams.saturated ? "true" : "false");
    if (!send_text(buffer)) {
        return;
    }
    for (unsigned int a = 0; a < ANALYTICS_POSITION_COUNT; ++a) {
        if (a && !send_text(",")) {
            return;
        }
        if (!send_text("[")) {
            return;
        }
        for (unsigned int b = 0; b < ANALYTICS_POSITION_COUNT; ++b) {
            snprintf(buffer, sizeof(buffer), "%s%u", b ? "," : "",
                     (unsigned int)usb_bigrams.counts[a][b]);
            if (!send_text(buffer)) {
                return;
            }
        }
        if (!send_text("]")) {
            return;
        }
    }
    send_text("]}\n");
}

static void send_recent(void) {
    zmk_analytics_recent_copy(&usb_recent);
    char buffer[160];
    snprintf(buffer, sizeof(buffer),
             "{\"protocol\":\"zmk-analytics-poc\",\"version\":3,\"kind\":\"recent\","
             "\"positions\":%u,\"capacity\":%u,\"next_sequence\":%lu,\"count\":%u,"
             "\"events\":[",
             (unsigned int)ANALYTICS_POSITION_COUNT, (unsigned int)ANALYTICS_RECENT_CAPACITY,
             (unsigned long)usb_recent.next_sequence, (unsigned int)usb_recent.count);
    if (!send_text(buffer)) {
        return;
    }
    for (unsigned int i = 0; i < usb_recent.count; ++i) {
        const struct analytics_recent_event *event = analytics_recent_get(&usb_recent, i);
        if (!event) {
            break;
        }
        snprintf(buffer, sizeof(buffer), "%s[%lu,%lu,%u,%u]", i ? "," : "",
                 (unsigned long)event->sequence, (unsigned long)event->timestamp_ms,
                 (unsigned int)event->position, (unsigned int)event->source);
        if (!send_text(buffer)) {
            return;
        }
    }
    send_text("]}\n");
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
            while (uart_poll_in(serial, &byte) == 0) {
            }
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
            } else if (!overflow && strcmp(command, "BIGRAMS") == 0) {
                send_bigrams();
            } else if (!overflow && strcmp(command, "RECENT") == 0) {
                send_recent();
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
