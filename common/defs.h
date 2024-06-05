#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <complex>
#include <vector>
#include <queue>
#include <functional>

enum class Method {
    SAX,
    CACHE,
    KMEANS,
    GRAPH,
};


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

class Node;
#ifdef ED
static int func_type = 0;
#define calc_dis(dim, data, query)
#endif
#ifdef L1
static int func_type = 1;
#define calc_dis(dim, data, query) l1_distance(dim, data, query)
#endif
#ifdef L2
static int func_type = 2;
#define calc_dis(dim, data, query) l2_distance(dim, data, query)
typedef std::tuple<float, Node *, Node *, std::vector<float>> NodeTuple;   // node_dis node pre_node pq_dis
typedef std::pair<float, float *> AnsPair;
typedef std::priority_queue<NodeTuple, std::vector<NodeTuple>, std::greater<NodeTuple>> NodeHeap;
typedef std::priority_queue<AnsPair> AnsHeap;
#endif


/////////////////////////////// SAX //////////////////////////////////////
//typedef unsigned short sax_type;  // cardinality = 512
typedef unsigned char sax_type;  // cardinality = 256
#define CARDINALITY 256
#define BIT_CARDINALITY 8