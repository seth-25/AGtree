#pragma once
#include "db.h"
#include "Node.h"


class MixTree {
public:
    MixTree(DB* db_);

    void rangeSearch(float *query, float query_r, std::vector<float> &ans_dis);

    void rangeSearchCache(Node *&node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis);

    void selectPivot(GNode *node, float *query);

    void crackVCache(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackGCache(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);



    void knnSearch(float *query, int k, AnsHeap &ans_dis);

    void knnSearchCache(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap);

    void addAns(int k, float dis, float *data, AnsHeap &ans_heap);

    void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap);

    void knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis);


    DB* db;
    Node* root;

    int max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;

    // temporary variable, storing intermediate results
    std::vector<float> distance;
    std::vector<int> pivot_pos;
    std::vector<std::vector<float>> pivot_dis;

};

