#ifndef EDIT_DISTANCE_UNWEIGHTED_COST_H
#define EDIT_DISTANCE_UNWEIGHTED_COST_H

#include <stdint.h>
#include <stdlib.h>
#include "utf8/utf8.h"

#endif

#ifndef COST_TYPE
#error "Must define COST_TYPE
#endif

#ifndef COST_NAME
#define COST_NAME_DEFINED
#define COST_NAME COST_TYPE
#endif

#define CONCAT3_(a, b, c) a ## b ## c
#define CONCAT3(a, b, c) CONCAT3_(a, b, c)
#define EDIT_DIST_TYPED(name) CONCAT3(edit_distance_, COST_NAME, _##name)

typedef struct {
    COST_TYPE insert_cost;
    COST_TYPE delete_cost;
    COST_TYPE replace_cost;
    COST_TYPE transpose_cost;
    bool case_insensitive;
} EDIT_DIST_TYPED(costs_t);

static const EDIT_DIST_TYPED(costs_t) EDIT_DIST_TYPED(default_costs) = {
    .insert_cost = (COST_TYPE) 1,
    .delete_cost = (COST_TYPE) 1,
    .replace_cost = (COST_TYPE) 1,
    .transpose_cost = (COST_TYPE) 1,
    .case_insensitive = true,
};

size_t EDIT_DIST_TYPED(cost_core)(const char *s1, size_t m, const char *s2, size_t n, bool reverse, COST_TYPE *costs, size_t costs_size, EDIT_DIST_TYPED(costs_t) op_costs) {
    const uint8_t *s1_ptr = (const uint8_t *) s1;
    const uint8_t *s2_ptr;
    size_t s1_len, s2_len;
    size_t s1_consumed = 0;
    size_t s2_consumed = 0;

    if (reverse) {
        s1_len = m;
        s2_len = n;
    }

    COST_TYPE *prev_costs = costs + (costs_size / 2);
    s2_consumed = 0;

    int32_t c1, c2, prev_c1, prev_c2;
    size_t i = 1;
    size_t used = 0;

    s2_ptr = (const uint8_t *) s2;
    s2_consumed = 0;
    costs[0] = (COST_TYPE) 0;
    size_t j = 1;
    while (s2_consumed < n) {
        c2 = 0;
        ssize_t c2_len = c2_len = utf8proc_iterate(s2_ptr, -1, &c2);
        if (c2 == 0 || c2_len == 0) break;
        costs[j] = j * op_costs.insert_cost;
        s2_ptr += c2_len;
        s2_consumed += c2_len;
        j++;
    }
    used = j; 

    while (s1_consumed < m) {
        c1 = 0;
        ssize_t c1_len = 0;
        if (!reverse) {
            c1_len = utf8proc_iterate(s1_ptr, -1, &c1);
        } else {
            c1_len = utf8proc_iterate_reversed(s1_ptr, s1_len - s1_consumed, &c1);
        }
        if (c1 == 0 || c1_len <= 0) break;
        if (op_costs.case_insensitive) c1 = utf8proc_tolower(c1);

        COST_TYPE prev_row_prev2_cost;
        COST_TYPE prev_row_prev_cost = costs[0];
        costs[0] += op_costs.delete_cost;

        s2_ptr = (const uint8_t *) s2;
        s2_consumed = 0;
        size_t j = 1;
        while (s2_consumed < n) {
            c2 = 0;
            ssize_t c2_len = 0;
            if (!reverse) {
                c2_len = utf8proc_iterate(s2_ptr, -1, &c2);
            } else {
                c2_len = utf8proc_iterate_reversed(s2_ptr, s2_len - s2_consumed, &c2);
            }
            if (c2 == 0 || c2_len == 0) break;
            if (op_costs.case_insensitive) c2 = utf8proc_tolower(c2);
            COST_TYPE v1 = costs[j] + op_costs.delete_cost;
            COST_TYPE v2 = costs[j - 1] + op_costs.insert_cost;
            COST_TYPE v3 = (c1 == c2) ? prev_row_prev_cost : prev_row_prev_cost + op_costs.replace_cost;

            COST_TYPE min_val = v1;
            if (v2 < min_val) min_val = v2;
            if (v3 < min_val) min_val = v3;

            if (j > 1 && i > 1 && c1 != c2 && c1 == prev_c2 && prev_c1 == c2) {
                COST_TYPE v4 = prev_row_prev2_cost + op_costs.transpose_cost;
                if (v4 <= min_val) min_val = v4;
            }

            prev_row_prev2_cost = prev_costs[j];
            prev_costs[j] = prev_row_prev_cost;
            prev_row_prev_cost = costs[j];

            costs[j] = min_val;

            if (!reverse) {
                s2_ptr += c2_len;
            }
            s2_consumed += c2_len;
            prev_c2 = c2;
            j++;
        }
        used = j;
        if (!reverse) {
            s1_ptr += c1_len;
        }
        s1_consumed += c1_len;
        prev_c1 = c1;
        i++;
    }

    return used;
}

size_t EDIT_DIST_TYPED(cost)(const char *s1, size_t m, const char *s2, size_t n, bool reverse, COST_TYPE *costs, size_t costs_size, void *options) {
    return EDIT_DIST_TYPED(cost_core)(s1, m, s2, n, reverse, costs, costs_size, *((EDIT_DIST_TYPED(costs_t) *)options) );
}

#undef CONCAT3_
#undef CONCAT3
#ifdef COST_NAME_DEFINED
#undef COST_NAME
#undef COST_NAME_DEFINED
#endif
