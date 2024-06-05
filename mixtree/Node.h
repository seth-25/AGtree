#pragma once

#include <vector>

enum class NodeType {
    VNode,
    GNode,
    LeafNode
};


class Node {
public:
    Node(int start_, int end_, NodeType type_, bool is_leaf_) :
    start(start_), end(end_), type(type_), is_leaf(is_leaf_) {};

    int start, end; // range of point id
    NodeType type;
    bool is_leaf;
    bool in_graph = false;
};

class VNode : public Node {
public:
    VNode(int start_, int end_) :  // internal node
            Node(start_, end_, NodeType::VNode, false) {}

    VNode(int start_, int end_, std::vector<float> &cache_dist_) :  // leaf node
            Node(start_, end_, NodeType::VNode, true) {
        cache_dis.swap(cache_dist_);
    }

    float *pivot;   // only one pivot
    float pivot_r;  // radius of pivot
    Node* left_child;
    Node* right_child;

    // leaf node
    std::vector<float> cache_dis; // 到支枢点的距离，只在叶子缓存
};


class GNode : public Node {
public:
    GNode(int start_, int end_, int pivot_cnt_) :  // internal node
            Node(start_, end_, NodeType::GNode, false), pivot_cnt(pivot_cnt_) {}

    GNode(int start_, int end_, int pivot_cnt_, std::vector<std::vector<float>> &cache_dist_) :  // leaf node
            Node(start_, end_, NodeType::GNode, true), pivot_cnt(pivot_cnt_) {
        cache_dis.swap(cache_dist_);
    }

    std::vector<float *> pivots;
    int pivot_cnt;
    std::vector<Node *> children;

    std::vector<std::vector<float>> min_dis;   // min_dis[a][b]：a到b内所有obj中最近的obj的距离
    std::vector<std::vector<float>> max_dis;   // max_dis[a][b]：a到b内所有obj中最远的obj的距离


    // leaf node
    std::vector<std::vector<float>> cache_dis; // 各个数据到各个支枢点的距离，只在叶子缓存，cache_dis[a][b]代表第a个数据到第b个支枢点的距离
};

class LeafNode: public Node {
public:
    float *pivot;
    std::vector<float> cache_dis; // 到支枢点的距离，只在叶子缓存
    bool in_graph = false;
};