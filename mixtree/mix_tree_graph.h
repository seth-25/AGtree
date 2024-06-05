#pragma once

#include "mix_tree_kmeans.h"
#include "../hnswlib/hnswalg.h"

class MixTreeGraph : public MixTreeKmeans {

public:
    explicit MixTreeGraph(DB* db_);

    void knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) override;

    void addGraph(Node *node);

    int M;
    int efConstruction;
    hnswlib::HierarchicalNSW<float>* alg_hnsw;
    std::vector<Node*> graph_nodes;

};