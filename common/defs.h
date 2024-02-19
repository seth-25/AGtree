#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <complex>

enum class Method {
    STANDARD,
    CACHE,
};

#ifdef ED
static int func_type = 0;
#endif
#ifdef L1
static int func_type = 1;
#endif
#ifdef L2
static int func_type = 2;
#endif




inline float l1_distance(int dim, const float *x, const float *y) {
    float ans = 0, res;
    for (int i = 0; i < dim; i++) {
        res = (x[i] - y[i]);
        if (res < 0) res = -res;
        ans += res;
    }
    return ans;
}

inline float l2_distance(int dim, const float *x, const float *y) {
    float ans = 0, res;
    int i;
    for (i = 0; i < dim; i++) {
        res = (x[i] - y[i]);
        ans += res * res;
    }
    return std::sqrt(ans);
}

#ifdef ED
#define dist(dim, data, query)
#endif
#ifdef L1
#define dist(dim, data, query) l1_distance(dim, data, query)
#endif
#ifdef L2
#define dist(dim, data, query) l2_distance(dim, data, query)
#endif