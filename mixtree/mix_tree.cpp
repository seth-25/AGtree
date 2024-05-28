#include <algorithm>
#include <cfloat>
#include "mix_tree.h"
#include "Node.h"
#include "record.h"

using namespace std;


MixTree::MixTree(DB *db_): db(db_) {
    avg_pivot_cnt = 5;
    max_pivot_cnt = min(2 * avg_pivot_cnt, 256);
    min_pivot_cnt = 2;
}

void MixTree::rangeSearch(float *query, float query_r, std::vector<float> &ans_dis) {
    rangeSearchImp(root, query, query_r, 0, ans_dis);
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


void MixTree::selectPivot(GNode *node, float* query) {
    //calc calc_dis between samples
    int sample_cnt = min(node->pivot_cnt * 3, node->end - node->start + 2); // data + query
    std::vector<std::vector<float>> pivot_dis(sample_cnt, vector<float>(sample_cnt));
    for (int i = 0; i < sample_cnt - 1; i ++ ) {
        for (int j = 0; j < sample_cnt - 1; j ++ ) {
//            if (i != j)
            pivot_dis[i][j] = pivot_dis[j][i] = calc_dis(db->dimension, db->data[node->start + i], db->data[node->start + j]);
//            else
//                pivot_dis[i][j] = pivot_dis[j][i] = 0;
        }
    }
    pivot_dis[sample_cnt - 1][sample_cnt - 1] = 0;
    for (int i = 0; i < sample_cnt - 1; i ++ ) {      // calc calc_dis between data and query
        pivot_dis[i][sample_cnt - 1] = pivot_dis[sample_cnt - 1][i] = calc_dis(db->dimension, db->data[node->start + i], query);
    }
    crack_calc_cnt += sample_cnt * sample_cnt;

    //select query as first pivot
    vector<bool> is_pivot(sample_cnt, false);
    std::vector<int> pivot_pos(node->pivot_cnt);
    int p = sample_cnt - 1;
    pivot_pos[0] = p;
    is_pivot[p] = true;

    // select pivots
    vector<float> min_dis(sample_cnt, FLT_MAX);
    for (int i = 1; i < node->pivot_cnt; i++) {
        for (int j = 0; j < sample_cnt; j++) {
            min_dis[j] = min(min_dis[j],  pivot_dis[j][pivot_pos[i - 1]]);   // min calc_dis between j and pivots which has selected
        }
        for (p = 0; is_pivot[p]; p++);    // initialize pivot p
        for (int j = p + 1; j < sample_cnt; j++) {
            if (min_dis[j] > min_dis[p] && !is_pivot[j]) {  // point that has max value of min calc_dis is the p
                p = j;
            }
        }
        pivot_pos[i] = p;
        is_pivot[p] = true;
    }
    node->pivots.emplace_back(query);
    for (int i = 1; i < node->pivot_cnt; i++) {
        node->pivots.emplace_back(db->data[node->start + pivot_pos[i]]);
    }
}



