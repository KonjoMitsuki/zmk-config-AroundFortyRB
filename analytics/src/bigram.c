/* SPDX-License-Identifier: MIT */
#include <string.h>
#include <analytics/bigram.h>

static uint16_t add16(uint16_t a, uint16_t b, bool *saturated) {
    if (a == UINT16_MAX) {
        *saturated = true;
        return UINT16_MAX;
    }
    uint32_t sum = (uint32_t)a + b;
    if (sum >= UINT16_MAX) {
        *saturated = true;
        return UINT16_MAX;
    }
    return (uint16_t)sum;
}

void analytics_bigrams_init(struct analytics_bigram_counts *bigrams) {
    memset(bigrams, 0, sizeof(*bigrams));
}

void analytics_bigram_record(struct analytics_bigram_counts *bigrams,
                             uint32_t previous, uint32_t position) {
    if (previous >= ANALYTICS_POSITION_COUNT || position >= ANALYTICS_POSITION_COUNT) {
        return;
    }
    uint16_t *value = &bigrams->counts[previous][position];
    if (*value == UINT16_MAX) {
        bigrams->saturated = true;
    } else {
        ++*value;
        if (*value == UINT16_MAX) {
            bigrams->saturated = true;
        }
    }
}

void analytics_bigrams_merge(struct analytics_bigram_counts *destination,
                             const struct analytics_bigram_counts *saved) {
    destination->saturated |= saved->saturated;
    for (unsigned int a = 0; a < ANALYTICS_POSITION_COUNT; ++a) {
        for (unsigned int b = 0; b < ANALYTICS_POSITION_COUNT; ++b) {
            destination->counts[a][b] =
                add16(destination->counts[a][b], saved->counts[a][b],
                      &destination->saturated);
        }
    }
}

bool analytics_bigrams_equal(const struct analytics_bigram_counts *a,
                             const struct analytics_bigram_counts *b) {
    return a->saturated == b->saturated &&
           memcmp(a->counts, b->counts, sizeof(a->counts)) == 0;
}
