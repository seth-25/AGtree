#pragma once

#include <unordered_map>
#include "db.h"
#include "Node.h"
#include "sax.h"


class MixTree {
public:
    explicit MixTree(DB* db_);

    void rangeSearch(float *query, float query_r, std::vector<float> &ans_dis);

    virtual void crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) = 0;

    virtual void crackG(Node *node, Node* pre_node, float *query, float query_r, std::vector<float> &ans_dis) = 0;

    virtual void rangeSearchImp(Node *node, Node* pre_node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis) = 0;


    void knnSearch(float *query, int k, AnsHeap &ans_dis);

    virtual void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) = 0;

    virtual void knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis) = 0;

    virtual void knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) = 0;


    void selectPivot(GNode *node, float *query);

    void addAns(int k, float dis, float *data, AnsHeap &ans_heap);


    DB* db;
    Node* root;

    int max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;

    // temporary variable, storing intermediate results
    std::vector<float> query_dist;
};

