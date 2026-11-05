/* SPDX-License-Identifier: MIT */
#include <string.h>
#include <analytics/collector.h>

static uint32_t add32(struct analytics_poc_snapshot *stats, uint32_t a, uint32_t b) {
    if (UINT32_MAX - a <= b) {
        stats->saturated = true;
        return UINT32_MAX;
    }
    return a + b;
}

static void increment(struct analytics_poc_snapshot *stats, uint32_t *counter) {
    if (*counter == UINT32_MAX) {
        stats->saturated = true;
    } else {
        ++*counter;
        if (*counter == UINT32_MAX) {
            stats->saturated = true;
        }
    }
}

static void add16(struct analytics_poc_snapshot *stats, uint16_t *value, uint32_t add) {
    if (*value == UINT16_MAX) {
        stats->histogram_saturated = true;
        return;
    }
    uint32_t sum = (uint32_t)*value + add;
    if (sum >= UINT16_MAX) {
        *value = UINT16_MAX;
        stats->histogram_saturated = true;
    } else {
        *value = (uint16_t)sum;
    }
}

static bool parse_uint(const char **cursor, uint32_t *value) {
    const char *p = *cursor;
    if (*p < '0' || *p > '9') {
        return false;
    }
    uint32_t result = 0;
    while (*p >= '0' && *p <= '9') {
        uint32_t digit = (uint32_t)(*p - '0');
        if (result > (UINT32_MAX - digit) / 10) {
            return false;
        }
        result = result * 10 + digit;
        ++p;
    }
    *cursor = p;
    *value = result;
    return true;
}

static int find_pair(const struct analytics_poc_snapshot *stats, uint32_t a, uint32_t b) {
    for (unsigned int i = 0; i < stats->pair_count; ++i) {
        if ((stats->pairs[i][0] == a && stats->pairs[i][1] == b) ||
            (stats->pairs[i][0] == b && stats->pairs[i][1] == a)) {
            return (int)i;
        }
    }
    return -1;
}

static void parse_pairs(struct analytics_poc_snapshot *stats, const char *list) {
    const char *p = list ? list : "";
    while (*p) {
        uint32_t a, b;
        if (!parse_uint(&p, &a) || *p != ':') {
            increment(stats, &stats->pair_config_errors);
        } else {
            ++p;
            if (!parse_uint(&p, &b) || (*p != ',' && *p != '\0')) {
                increment(stats, &stats->pair_config_errors);
            } else if (a >= ANALYTICS_POSITION_COUNT || b >= ANALYTICS_POSITION_COUNT || a == b) {
                increment(stats, &stats->pair_config_errors);
            } else {
                if (a > b) {
                    uint32_t temporary = a;
                    a = b;
                    b = temporary;
                }
                if (find_pair(stats, a, b) < 0) {
                    if (stats->pair_count < ANALYTICS_PAIR_MAX) {
                        stats->pairs[stats->pair_count][0] = (uint8_t)a;
                        stats->pairs[stats->pair_count][1] = (uint8_t)b;
                        ++stats->pair_count;
                    } else {
                        increment(stats, &stats->pair_config_errors);
                    }
                }
            }
        }
        while (*p && *p != ',') {
            ++p;
        }
        if (*p == ',') {
            ++p;
        }
    }
}

void analytics_poc_init(struct analytics_poc_snapshot *stats, const char *pair_list) {
    memset(stats, 0, sizeof(*stats));
    parse_pairs(stats, pair_list);
}

void analytics_poc_record(struct analytics_poc_snapshot *stats, uint32_t position,
                          uint8_t source, bool pressed) {
    if (position >= ANALYTICS_POSITION_COUNT) {
        increment(stats, &stats->invalid_positions);
        return;
    }

    enum analytics_source_class kind =
        source == UINT8_MAX ? ANALYTICS_LOCAL : ANALYTICS_PERIPHERAL;
    increment(stats, pressed ? &stats->presses[position][kind] : &stats->releases[kind]);
}

static uint16_t pair_near_scratch[ANALYTICS_PAIR_MAX][ANALYTICS_TIMING_CLASSES]
                                 [ANALYTICS_PAIR_BINS];
static uint16_t pair_overlap_scratch[ANALYTICS_PAIR_MAX][ANALYTICS_TIMING_CLASSES]
                                    [ANALYTICS_PAIR_BINS];

void analytics_poc_merge(struct analytics_poc_snapshot *destination,
                         struct analytics_poc_snapshot *saved) {
    destination->saturated |= saved->saturated;
    destination->histogram_saturated |= saved->histogram_saturated;
    for (unsigned int p = 0; p < ANALYTICS_POSITION_COUNT; ++p) {
        for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s) {
            destination->presses[p][s] =
                add32(destination, destination->presses[p][s], saved->presses[p][s]);
        }
        for (unsigned int b = 0; b < ANALYTICS_TIME_BINS; ++b) {
            add16(destination, &destination->hold[p][b], saved->hold[p][b]);
            add16(destination, &destination->same_key[p][b], saved->same_key[p][b]);
            add16(destination, &destination->idle_before_press[p][b],
                  saved->idle_before_press[p][b]);
        }
    }
    for (unsigned int s = 0; s < ANALYTICS_SOURCE_CLASSES; ++s) {
        destination->releases[s] =
            add32(destination, destination->releases[s], saved->releases[s]);
    }
    destination->invalid_positions =
        add32(destination, destination->invalid_positions, saved->invalid_positions);
    destination->duplicate_downs =
        add32(destination, destination->duplicate_downs, saved->duplicate_downs);
    destination->orphan_releases =
        add32(destination, destination->orphan_releases, saved->orphan_releases);
    destination->timing_discards =
        add32(destination, destination->timing_discards, saved->timing_discards);

    memset(pair_near_scratch, 0, sizeof(pair_near_scratch));
    memset(pair_overlap_scratch, 0, sizeof(pair_overlap_scratch));
    for (unsigned int si = 0; si < saved->pair_count && si < ANALYTICS_PAIR_MAX; ++si) {
        int di = find_pair(destination, saved->pairs[si][0], saved->pairs[si][1]);
        if (di < 0) {
            continue;
        }
        for (unsigned int c = 0; c < ANALYTICS_TIMING_CLASSES; ++c) {
            for (unsigned int b = 0; b < ANALYTICS_PAIR_BINS; ++b) {
                pair_near_scratch[di][c][b] = saved->pair_near[si][c][b];
                pair_overlap_scratch[di][c][b] = saved->pair_overlap[si][c][b];
                add16(destination, &destination->pair_near[di][c][b],
                      saved->pair_near[si][c][b]);
                add16(destination, &destination->pair_overlap[di][c][b],
                      saved->pair_overlap[si][c][b]);
            }
        }
    }
    /* Normalize the saved copy so it can be compared with the current pair
     * configuration after a restore. */
    saved->pair_count = destination->pair_count;
    saved->pair_config_errors = destination->pair_config_errors;
    memcpy(saved->pairs, destination->pairs, sizeof(saved->pairs));
    memcpy(saved->pair_near, pair_near_scratch, sizeof(saved->pair_near));
    memcpy(saved->pair_overlap, pair_overlap_scratch, sizeof(saved->pair_overlap));
}

bool analytics_stats_equal(const struct analytics_poc_snapshot *a,
                           const struct analytics_poc_snapshot *b) {
    return memcmp(a->presses, b->presses, sizeof(a->presses)) == 0 &&
           memcmp(a->releases, b->releases, sizeof(a->releases)) == 0 &&
           a->invalid_positions == b->invalid_positions &&
           a->duplicate_downs == b->duplicate_downs &&
           a->orphan_releases == b->orphan_releases &&
           a->timing_discards == b->timing_discards &&
           a->pair_config_errors == b->pair_config_errors &&
           a->saturated == b->saturated &&
           a->histogram_saturated == b->histogram_saturated &&
           a->pair_count == b->pair_count &&
           memcmp(a->pairs, b->pairs, sizeof(a->pairs)) == 0 &&
           memcmp(a->hold, b->hold, sizeof(a->hold)) == 0 &&
           memcmp(a->same_key, b->same_key, sizeof(a->same_key)) == 0 &&
           memcmp(a->idle_before_press, b->idle_before_press,
                  sizeof(a->idle_before_press)) == 0 &&
           memcmp(a->pair_near, b->pair_near, sizeof(a->pair_near)) == 0 &&
           memcmp(a->pair_overlap, b->pair_overlap, sizeof(a->pair_overlap)) == 0;
}
