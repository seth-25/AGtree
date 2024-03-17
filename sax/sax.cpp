//
// Created by seth on 2024/3/17.
//

#include "sax.h"

void paa_from_ts(const float *ts_in, float *paa, int segments, int num_per_segment) {
    assert(BIT_CARDINALITY == 8);
    for (int i = 0, s = 0; s < segments; i += num_per_segment, s++) {
        paa[s] = 0;
        for (int j = i; j < i + num_per_segment; j++) {
            paa[s] += ts_in[j];
        }
        paa[s] /= (float) num_per_segment;
    }
}

void sax_from_ts(const float *ts_in, sax_type* sax_out, int segments, int num_per_segment)
{
    assert(BIT_CARDINALITY == 8);
    float paa[segments];
    for (int i = 0, s = 0; s < segments; i += num_per_segment, s ++) {
        paa[s] = 0;
        for (int j = i; j < i + num_per_segment; j ++ ) {
            paa[s] += ts_in[j];
        }
        paa[s] /= (float)num_per_segment;
    }

    for (int s = 0; s < segments; s ++ ) {
        int l = 0, r = 256;
        while(l < r) {
            int mid = (l + r + 1) >> 1;
            if (sax_256[mid] < paa[s]) l = mid;
            else r = mid - 1;
        }
        sax_out[s] = l;
    }
}

float min_dist_paa_to_sax(const float *paa, const sax_type* sax_, int segments, int num_per_segment) {
    assert(BIT_CARDINALITY == 8);
    float dis = 0;
    for (int i = 0; i < segments; i ++ ) {
        sax_type region = sax_[i];
        float breakpoint_lower = sax_256[region];
        float breakpoint_upper = sax_256[region + 1];

        if (breakpoint_lower > paa[i]) {
            dis += (breakpoint_lower - paa[i]) * (breakpoint_lower - paa[i]);
        }
        else if(breakpoint_upper < paa[i]) {
            dis += (breakpoint_upper - paa[i]) * (breakpoint_upper - paa[i]);
        }
    }
    return sqrt(dis * (float)num_per_segment);
}