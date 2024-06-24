#include <queue>
#include <cfloat>
#include "mix_tree_kmeans.h"
#include "record.h"
#include "../dkm/dkm_parallel.hpp"

using namespace std;

MixTreeKmeans::MixTreeKmeans(DB *db_): MixTreeCache(db_) {
    sample_num = 100;
    max_iter = 30;

    query_dist.resize(db->num_data);
    vector<float> cache_dis(db->num_data, 0);
    root = new VNode(0, db->num_data - 1, cache_dis);
}


float MixTreeKmeans::chose_split_dis(Node* node, int num_cluster) {
    int rnd_num = min(sample_num, node->end - node->start + 1);
    vector<array<float, 1>> sample_rnd_dis(rnd_num);
    for (int i = 0; i < rnd_num; i ++ ) {
        sample_rnd_dis[i][0] = query_dist[node->start + i];
    }
    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd_dis, num_cluster, max_iter); // k = 2
    const auto& means = std::get<0>(k_res);
    const auto& labels = std::get<1>(k_res);
    int target_label = means[0][0] < means[1][0] ? 0 : 1;

//    cout << "start end " << node->start << " " << node->end << endl;
////    for (int i = node->start; i < node->start + rnd_num; i ++ ) {
////        cout << db->data[i][0] << " ";
////    }
////    cout << endl;
//    {   // todo delete
//        cout << "target_label:" << target_label << endl;
//        cout << "Means:";
//        for (const auto& mean : means) {
//            cout << "\t(" << mean[0] << ")";
//        }
//        cout << endl;
//    }


    float split_dis = 0;
    int cnt0 = 0, cnt1 = 0; // todo delete
    for (int i = 0; i < labels.size(); i ++ ) {
        const auto& label = labels[i];
        if (label == target_label) {
            split_dis = max(split_dis, sample_rnd_dis[i][0]);
            cnt0 ++;
        }
        else {
            cnt1 ++;
        }
    }

//    cout << "clusters:" << cnt0 << " " << cnt1 << endl; // todo delete
//    cout << "split_dis:" << split_dis << endl; // todo delete
    return split_dis;
}

void MixTreeKmeans::crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) {
//    cout << "kmeans crack V" << endl;
    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;
    vector<float>().swap(v_node->cache_dis);


//    sort(db->data + node->start, db->data + node->end + 1, [&](const float* a, const float* b) {    // todo del
//        return a[0] < b[0];
//    });

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    float split_dis = chose_split_dis(v_node, 2);

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(query_dist[l] <= split_dis && l <= v_node->end) {
            if (query_dist[l] <= query_r) {
                ans_dis.emplace_back(query_dist[l]);
            }
            l++;
        }
        while(query_dist[r] > split_dis && r >= v_node->start) {
            if (query_dist[r] <= query_r) {
                ans_dis.emplace_back(query_dist[r]);
            }
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(query_dist[l], query_dist[r]);
    }

//        vector<float> query_dist_copy;
//        query_dist_copy = query_dist;
//        sort(query_dist_copy.begin() + v_node->start, query_dist_copy.begin() + v_node->end + 1);
//        for (int i = v_node->start; i <= v_node->end; i ++ ) {
//            if (i % 10 == 0)
//            cout << i  << ":" << query_dist_copy[i] <<  ";;; ";
//        }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(query_dist.begin() + v_node->start, query_dist.begin() + r + 1, cache_left.begin());
    std::swap_ranges(query_dist.begin() + r + 1, query_dist.begin() + v_node->end + 1, cache_right.begin());

//    cout << "split num:" << cache_left.size() << " " << cache_right.size() << endl; // todo delete


    v_node->pivot = query;
    v_node->pivot_r = split_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTreeKmeans::crackG(Node *node, Node* pre_node, float *query, float query_r, vector<float>& ans_dis) {
    GNode* g_node;
    if (node->type == NodeType::GNode) {
        node->is_leaf = false;
        g_node = (GNode*)node;
    }
    else {  // VNode to GNode
        g_node = new GNode(node->start, node->end, max_pivot_cnt);
        if (node != root) {
            VNode* v_node = (VNode*) pre_node;
            if (node == v_node->left_child) {
                v_node->left_child = g_node;
            }
            else if (node == v_node->right_child) {
                v_node->right_child = g_node;
            }
            else {
                cout << "crack g error" << endl;
                exit(255);
            }
            delete node;
        }
        else {
            root = g_node;
        }
    }
    vector<vector<float>>().swap(g_node->cache_dis);

    selectPivot(g_node, query);

//    sort(db->data + g_node->start, db->data + g_node->end + 1, [&](const float* a, const float* b) {    // todo del
//        return a[0] < b[0];
//    });
    for (int i = g_node->start; i <= g_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += g_node->end - g_node->start + 1;

//    float split_dis = chose_split_dis(g_node, g_node->pivot_cnt);
    float split_dis = chose_split_dis(g_node, 2);

    int pivot_cnt = g_node->pivot_cnt;
    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
//    cout << "G " << pivot_cnt << endl;
    vector<float> tmp_dis(pivot_cnt);   // 当前数据到各个支枢点的距离，tmp_dis[i]表示到第i个支枢点的距离
    vector<vector<int>> pivot_data(pivot_cnt);  // 分配给各个支枢点的数据的id，pivot_data[i]代表分配给第i个支枢点的数据的id
    vector<vector<vector<float>>> pivot_data_dist(pivot_cnt);    // 数据到分配的支枢点的距离，pivot_data_dist[i][x][j]代表分配给第i个支枢点的数据x，到第j个支枢点的距离

    int data_cnt = g_node->end - g_node->start + 1;
    for (int i = 0; i < data_cnt; i ++ ) {
        int id = i + g_node->start;
        tmp_dis[0] = query_dist[id];    // 数据到第0个支枢点(query)的距离
        if (tmp_dis[0] <= query_r) {
            ans_dis.emplace_back(tmp_dis[0]);
        }
        for (int j = 1; j < pivot_cnt; j ++ ) {
            tmp_dis[j] = calc_dis(db->dimension, db->data[id], g_node->pivots[j]);   // 数据到第j个支枢点的距离
        }
        crack_calc_cnt += pivot_cnt - 1;

        int closest_pivot;
        if (tmp_dis[0] <= split_dis) {  // add data to pivot-query
            closest_pivot = 0;
        }
        else {
            closest_pivot = (int)(min_element(tmp_dis.begin() + 1, tmp_dis.end()) - tmp_dis.begin());
        }

        pivot_data[closest_pivot].emplace_back(id);
        pivot_data_dist[closest_pivot].emplace_back(tmp_dis);
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

//    cout << "split num:";
//    for (int i = 0; i < pivot_data.size(); i ++ ) {
//        cout << pivot_data[i].size() << " ";
//    }
//    cout << "| " << g_node->end - g_node->start + 1 << endl;
}


void MixTreeKmeans::knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) {
    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;
    vector<float>().swap(v_node->cache_dis);

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    float split_dis = chose_split_dis(v_node, 2);

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(query_dist[l] <= split_dis && l <= v_node->end) {
            addAns(k, query_dist[l], db->data[l], ans_heap);
            l++;
        }
        while(query_dist[r] > split_dis && r >= v_node->start) {
            addAns(k, query_dist[r], db->data[r], ans_heap);
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(query_dist[l], query_dist[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(query_dist.begin() + v_node->start, query_dist.begin() + r + 1, cache_left.begin());
    std::swap_ranges(query_dist.begin() + r + 1, query_dist.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = split_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTreeKmeans::knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis) {
    GNode* g_node;
    if (node->type == NodeType::GNode) {
        node->is_leaf = false;
        g_node = (GNode*)node;
    }
    else {
        g_node = new GNode(node->start, node->end, max_pivot_cnt);
        if (node != root) {
            VNode* v_node = (VNode*) pre_node;
            if (node == v_node->left_child) {
                v_node->left_child = g_node;
            }
            else if (node == v_node->right_child) {
                v_node->right_child = g_node;
            }
            else {
                cout << "crack g error" << endl;
                exit(255);
            }
            delete node;
        }
        else {
            root = g_node;
        }
    }
    vector<vector<float>>().swap(g_node->cache_dis);

    int pivot_cnt = g_node->pivot_cnt;
//    cout << "pivot cnt:" << pivot_cnt << endl;
    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
    selectPivot(g_node, query);

    for (int i = g_node->start; i <= g_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += g_node->end - g_node->start + 1;
    float split_dis = chose_split_dis(g_node, 2);

    vector<float> tmp_dis(pivot_cnt);   // 当前数据到各个支枢点的距离，tmp_dis[i]表示到第i个支枢点的距离
    vector<vector<int>> pivot_data(pivot_cnt);  // 分配给各个支枢点的数据的id，pivot_data[i]代表分配给第i个支枢点的数据的id
    vector<vector<vector<float>>> pivot_data_dist(pivot_cnt);    // 数据到分配的支枢点的距离，pivot_data_dist[i][x][j]代表分配给第i个支枢点的数据x，到第j个支枢点的距离
    int data_cnt = g_node->end - g_node->start + 1;
    for (int i = 0; i < data_cnt; i ++ ) {
        int id = i + g_node->start;
        tmp_dis[0] = query_dist[id];
        addAns(k, tmp_dis[0], db->data[id], ans_dis);
        for (int j = 1; j < pivot_cnt; j ++ ) {
            tmp_dis[j] = calc_dis(db->dimension, db->data[id], g_node->pivots[j]);   // 当前数据到第j个支枢点的距离
        }
        crack_calc_cnt += pivot_cnt - 1;

        int closest_pivot ;
        if (tmp_dis[0] <= split_dis) {
            closest_pivot = 0;
        }
        else {
            closest_pivot= (int)(min_element(tmp_dis.begin(), tmp_dis.end()) - tmp_dis.begin());
        }

        pivot_data[closest_pivot].emplace_back(id);
        pivot_data_dist[closest_pivot].emplace_back(tmp_dis);
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