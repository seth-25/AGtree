#include <cfloat>
#include <algorithm>
#include "MixTree.h"
#include "record.h"

using namespace std;


void MixTree::selectPivot(GNode *node, float* query) {
    //calc calc_dis between samples
    int sample_cnt = min(node->pivot_cnt * 3, node->end - node->start + 2); // data + query
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

void MixTree::crackGCache(Node *&node, float *query, float query_r, vector<float>& ans_dis) {
    vector<float>().swap(node->cache_dis);
//    node->cache_dis.clear();
//    node->cache_dis.shrink_to_fit();

    GNode* g_node;
    if (node->type == NodeType::GNode) {
        node->is_leaf = false;
        g_node = (GNode*)node;
    }
    else {
        g_node = new GNode(node->start, node->end, max_pivot_cnt);
        delete node;
        node = g_node;
    }
    int pivot_cnt = g_node->pivot_cnt;
    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
    selectPivot(g_node, query);

    vector<float> tmp_dis(pivot_cnt);
    vector<vector<int>> pivot_data(pivot_cnt);  // 分配给各个支枢点的数据的id
    vector<vector<float>> pivot_data_dist(pivot_cnt);    // 数据到分配的支枢点的距离
    int data_cnt = g_node->end - g_node->start + 1;
    for (int i = 0; i < data_cnt; i ++ ) {
        for (int j = 0; j < pivot_cnt; j ++ ) {
            tmp_dis[j] = calc_dis(db->dimension, db->data[g_node->start + i], g_node->pivots[j]);   // 数据到第j个支枢点的距离
            if (j == 0) {   // pivot is query
                if (tmp_dis[j] < query_r) {
                    ans_dis.emplace_back(tmp_dis[j]);
                }
                search_calc_cnt ++;
            }
            else {
                crack_calc_cnt ++;
            }
        }
        int closest_pivot = (int)(min_element(tmp_dis.begin(), tmp_dis.end()) - tmp_dis.begin());
        pivot_data[closest_pivot].emplace_back(g_node->start + i);
        pivot_data_dist[closest_pivot].emplace_back(tmp_dis[closest_pivot]);
        for (int j = 0; j < pivot_cnt; j ++ ) {
            g_node->max_dis[j][closest_pivot] = max(g_node->max_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最远的obj的距离
            g_node->min_dis[j][closest_pivot] = min(g_node->min_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最近的obj的距离
        }
    }

    float *data_tmp[data_cnt];
    int tmp_len = 0;
    for (int i = 0; i < pivot_cnt; i++) {
        int new_node_start = tmp_len + node->start;
        for (int data_id: pivot_data[i]) {
            data_tmp[tmp_len] = db->data[data_id];
            tmp_len++;
        }
        // create new node
        int new_node_end = tmp_len + node->start - 1;
        int next_pivot_cnt = (int)pivot_data[i].size() * avg_pivot_cnt * pivot_cnt / data_cnt;
        next_pivot_cnt = max(min_pivot_cnt, next_pivot_cnt);
        next_pivot_cnt = min(max_pivot_cnt, next_pivot_cnt);
        GNode* leaf_node = new GNode(new_node_start, new_node_end, next_pivot_cnt, pivot_data_dist[i]);
        g_node->children.emplace_back(leaf_node);
    }
    for (int i = g_node->start; i <= g_node->end; i ++ ) {
        db->data[i] = data_tmp[i - g_node->start];
    }
}

void MixTree::knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis) {
    vector<float>().swap(node->cache_dis);
//    node->cache_dis.clear();
//    node->cache_dis.shrink_to_fit();

    GNode* g_node;
    if (node->type == NodeType::GNode) {
        node->is_leaf = false;
        g_node = (GNode*)node;
    }
    else {
        cout << "delete" << endl;
        g_node = new GNode(node->start, node->end, max_pivot_cnt);
        if (node != root) {
            VNode* v_node = (VNode*) pre_node;
            if (node == v_node->left_child) {
                cout << "is left" << endl;
                v_node->left_child = g_node;
            }
            else if (node == v_node->right_child) {
                cout << "is right" << endl;
                v_node->right_child = g_node;
            }
            else {
                cout << "error" << endl;
                exit(255);
            }
            delete node;
        }
        else {
            root = g_node;
        }
    }
    int pivot_cnt = g_node->pivot_cnt;
//    cout << "pivot cnt:" << pivot_cnt << endl;
    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
    selectPivot(g_node, query);

    vector<float> tmp_dis(pivot_cnt);
    vector<vector<int>> pivot_data(pivot_cnt);  // 分配给各个支枢点的数据的id
    vector<vector<float>> pivot_data_dist(pivot_cnt);    // 数据到分配的支枢点的距离
    int data_cnt = g_node->end - g_node->start + 1;
    for (int i = 0; i < data_cnt; i ++ ) {
        for (int j = 0; j < pivot_cnt; j ++ ) {
            tmp_dis[j] = calc_dis(db->dimension, db->data[g_node->start + i], g_node->pivots[j]);   // 数据到第j个支枢点的距离
            if (j == 0) {   // pivot is query
                addAns(k, tmp_dis[j], db->data[g_node->start + i], ans_dis);
                search_calc_cnt ++;
            }
            else {
                crack_calc_cnt ++;
            }
        }
        int closest_pivot = (int)(min_element(tmp_dis.begin(), tmp_dis.end()) - tmp_dis.begin());
        pivot_data[closest_pivot].emplace_back(g_node->start + i);
        pivot_data_dist[closest_pivot].emplace_back(tmp_dis[closest_pivot]);
        for (int j = 0; j < pivot_cnt; j ++ ) {
            g_node->max_dis[j][closest_pivot] = max(g_node->max_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最远的obj的距离
            g_node->min_dis[j][closest_pivot] = min(g_node->min_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最近的obj的距离
        }
    }

    float *data_tmp[data_cnt];
    int tmp_len = 0;
    for (int i = 0; i < pivot_cnt; i++) {
        int new_node_start = tmp_len + g_node->start;
        for (int data_id: pivot_data[i]) {
            data_tmp[tmp_len] = db->data[data_id];
            tmp_len++;
        }
        // create new node
        int new_node_end = tmp_len + g_node->start - 1;
        int next_pivot_cnt = (int)pivot_data[i].size() * avg_pivot_cnt * pivot_cnt / data_cnt;
        next_pivot_cnt = max(min_pivot_cnt, next_pivot_cnt);
        next_pivot_cnt = min(max_pivot_cnt, next_pivot_cnt);
        GNode* leaf_node = new GNode(new_node_start, new_node_end, next_pivot_cnt, pivot_data_dist[i]);
        g_node->children.emplace_back(leaf_node);
    }
    for (int i = g_node->start; i <= g_node->end; i ++ ) {
        db->data[i] = data_tmp[i - g_node->start];
    }
}