#pragma once
#include "mix_tree_cache.h"

class MixTreeKmeans : public MixTreeCache {
public:
    explicit MixTreeKmeans(DB *db_);

//    void selectPivot(GNode *node, float *query) override;

    float chose_split_dis(Node *node, int num_cluster);

    void crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) override;

    void crackG(Node *node, Node* pre_node, float *query, float query_r, std::vector<float>& ans_dis) override;

    void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) override;

    void knnCrackG(Node *node, Node *pre_node, float *query, int k, AnsHeap &ans_dis);

    int sample_num, max_iter;   // kmeans parm
};
