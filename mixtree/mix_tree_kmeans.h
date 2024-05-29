#pragma once
#include "mix_tree_cache.h"

class MixTreeKmeans : public MixTreeCache {
public:
    explicit MixTreeKmeans(DB *db_);

    void crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) override;

    void rangeSearchImp(Node *node, Node* pre_node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis) override;

    void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) override;

    void knnSearchImp(float *query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) override;


    int sample_num, max_iter;   // kmeans parm
};
