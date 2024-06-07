#pragma once

#include "mix_tree_kmeans.h"
#include "../hnswlib/hnswalg.h"

class MixTreeGraph : public MixTreeKmeans {

public:
    explicit MixTreeGraph(DB* db_);

    ~MixTreeGraph();

    void knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) override;

    void addGraph(Node *node);

    void approximateKnnSearch(float *query, int k, NodeHeap &node_heap, AnsHeap &ans_heap, std::vector<std::pair<float, size_t>>& node_pairs);

    void exactKnnSearch(float *query, int k, NodeHeap &node_heap, AnsHeap &ans_heap);

    int M;
    int efConstruction;
    hnswlib::L2Space* l2space;
    hnswlib::HierarchicalNSW<float>* alg_hnsw;
    std::vector<Node*> graph_nodes;
};