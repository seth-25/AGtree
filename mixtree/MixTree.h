#pragma once
#include "db.h"
#include "Node.h"


class MixTree {
public:
    MixTree(DB* db_);

    void search(float *query, float query_r, std::vector<float> &ans_dis);

    void selectPivot(GNode *node, float *query);

    void crackVCache(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackGInManyCache(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void searchCache(Node *&node, float *query, float query_r, float pq_dist, std::vector<float> &ans_dis);

    DB* db;
    Node* root;

    int max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;

    // temporary variable, storing intermediate results
    std::vector<float> distance;
    std::vector<int> pivot_pos;
    std::vector<std::vector<float>> pivot_dis;

};

