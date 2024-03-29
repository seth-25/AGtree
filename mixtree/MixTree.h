#pragma once

#include <unordered_map>
#include "db.h"
#include "Node.h"
#include "sax.h"


class MixTree {
public:
    MixTree(DB* db_);

    ~MixTree();

    void selectPivot(GNode *node, float *query);

    void crackV(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackVSax(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackVKmeans(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackG(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);

    void crackGSax(Node *&node, float *query, float query_r, std::vector<float> &ans_dis);




    void rangeSearch(float *query, float query_r, std::vector<float> &ans_dis);

    void rangeSearchCache(Node *&node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis);

    void rangeSearchSax(Node *&node, float *query, float query_r, float *query_paa, float pq_dis, std::vector<float> &ans_dis);

    void rangeSearchKmeans(Node *&node, float *query, float query_r, float pq_dis, std::vector<float> &ans_dis);




    void addAns(int k, float dis, float *data, AnsHeap &ans_heap);

    void knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap);

    void knnCrackVKmeans(Node *node, float *query, int k, AnsHeap &ans_heap);

    void knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis);

    void knnSearch(float *query, int k, AnsHeap &ans_dis);

    void knnSearch(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap);

    void knnSearchSax(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap);

    void knnSearchKmeans(float *query, int k, NodeHeap &node_heap, AnsHeap &ans_heap);





    DB* db;
    Node* root;

    int max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;
    int sample_num, max_iter;   // kmeans parm

    // temporary variable, storing intermediate results
    std::vector<float> distance;
    std::vector<sax_type*> saxes;

};

