/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <analytics/collector.h>

/* Directed consecutive-press counts. counts[a][b] is the number of observed
 * transitions from position a to position b, including a == b. This is the
 * sufficient statistic for a transition-weight dashboard; it does not retain
 * the full press sequence. */
struct analytics_bigram_counts {
    uint16_t counts[ANALYTICS_POSITION_COUNT][ANALYTICS_POSITION_COUNT];
    bool saturated;
};

void analytics_bigrams_init(struct analytics_bigram_counts *bigrams);
void analytics_bigram_record(struct analytics_bigram_counts *bigrams,
                             uint32_t previous, uint32_t position);
void analytics_bigrams_merge(struct analytics_bigram_counts *destination,
                             const struct analytics_bigram_counts *saved);
bool analytics_bigrams_equal(const struct analytics_bigram_counts *a,
                             const struct analytics_bigram_counts *b);
