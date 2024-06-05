#include <algorithm>
#include <cfloat>
#include "mix_tree.h"
#include "Node.h"
#include "record.h"

#include "mix_tree_graph.h"

using namespace std;


MixTree::MixTree(DB *db_): db(db_) {
    avg_pivot_cnt = 3;
    max_pivot_cnt = 3;
    min_pivot_cnt = 2;
}

void MixTree::rangeSearch(float *query, float query_r, std::vector<float> &ans_dis) {
    rangeSearchImp(root, nullptr, query, query_r, vector<float>(1, 0), ans_dis);
}

void MixTree::knnSearch(float *query, int k, AnsHeap &ans_dis) {
    NodeHeap node_heap;
    node_heap.emplace(0, root, nullptr, 0);
    knnSearchImp(query, k, node_heap, ans_dis);
}


void MixTree::addAns(int k, float dis, float* data, AnsHeap &ans_heap) {
    if (ans_heap.size() < k) {
        ans_heap.emplace(dis, data);
    }
    else if (dis < ans_heap.top().first) {
        ans_heap.pop();
        ans_heap.emplace(dis, data);
    }
}





