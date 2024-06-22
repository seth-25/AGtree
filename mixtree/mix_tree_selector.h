#pragma once
#include "mix_tree_kmeans.h"

class MixTreeSelector : public MixTreeKmeans {
public:
    explicit MixTreeSelector(DB *db_);

    void checkCrack(float *query);

    void rangeSearchImp(Node *node, Node *pre_node, float *query, float query_r, std::vector<float> pq_dis,
                        std::vector<float> &ans_dis);

    void rangeSearchImpRec(Node *node, Node *pre_node, float *query, float query_r, std::vector<float> pq_dis,
                           std::vector<float> &ans_dis);

private:
    struct selector_t {
        float* pivot;
        int cnt; // count of query
        int id; // idx in fibonacci

        selector_t(float* pivot_, int cnt_, int id_) : pivot(pivot_), cnt(cnt_), id(id_) {}
    };
    std::vector<selector_t> selector;
//    const int fibonacci[25] = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144,
//                               233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
    const int fibonacci[100] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
                               2, 2, 4, 4, 4, 4, 8, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    bool need_crack = false;

};
