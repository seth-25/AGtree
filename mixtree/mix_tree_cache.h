#pragma once

#include "mix_tree.h"

class MixTreeCache : public MixTree {
public:
    explicit MixTreeCache(DB* db_);

    ~MixTreeCache();

    void crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) override;

    void crackG(Node *node, Node* pre_node, float *query, float query_r, std::vector<float> &ans_dis) override;

    void rangeSearchImp(Node *node, Node* pre_node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis) override;


    void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) override;

    void knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis) override;

    void knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) override;
};

