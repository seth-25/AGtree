#pragma once
#include <vector>
#include "db.h"

class Node {
public:
    Node(int start_, int end_, int pivot_cnt_) :
            start(start_), end(end_), pivot_cnt(pivot_cnt_) {};

    Node(int start_, int end_, int pivot_cnt_, std::vector<float>& cache_dist_) :
            start(start_), end(end_), pivot_cnt(pivot_cnt_) {
        cache_dist.swap(cache_dist_);
    };

    // internal node
    int start, end; // range of point id
    int pivot_cnt;
    std::vector<float*> pivots;
    std::vector<Node*> children;

    std::vector<std::vector<float>> min_dist;   // min_dist[a][b]：a到b内所有obj中最近的obj的距离
    std::vector<std::vector<float>> max_dist;   // max_dist[a][b]：a到b内所有obj中最远的obj的距离

    bool is_leaf = true;

    // leaf node
    std::vector<float> cache_dist; // 到父亲支枢点的距离，只在叶子缓存
};

class AGtree {
private:
    DB* db;

public:
    AGtree(DB* db_);

    void crackInTwo(Node *node, float *query, float query_r, float *&dis, std::vector<float> &ans_dis);

    void crackInMany(Node *node, float *query, float query_r, std::vector<float> &ans_dis);

    void search(Node *node, float *query, float query_r, float *&distance, std::vector<float> &ans_dis);

    void selectPivot(Node *node, float *query);

    void crackInManyCache(Node *node, float *query, float query_r, std::vector<float> &ans_dis);

    void
    searchCache(Node *node, float *query, float query_r, float pq_dist, float *&distance, std::vector<float> &ans_dis);

    void
    searchMany(Node *node, float *query, float query_r, float *&distance, std::vector<float> &ans_dis);

    Node* root;
    int max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;
    std::vector<int> pivot_pos;
    std::vector<std::vector<float>> pivot_dis;
};