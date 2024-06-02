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


float MixTreeKmeans::chose_split_dis(Node* node) {
    int rnd_num = min(sample_num, node->end - node->start + 1);
    vector<array<float, 1>> sample_rnd_dis(rnd_num);
    for (int i = 0; i < rnd_num; i ++ ) {
        sample_rnd_dis[i][0] = query_dist[node->start + i];
    }
    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd_dis, 2, max_iter); // k = 2
    const auto& means = std::get<0>(k_res);
    const auto& labels = std::get<1>(k_res);
    int target_label = means[0][0] < means[1][0] ? 0 : 1;

    cout << "start end " << node->start << " " << node->end << endl;
    for (int i = node->start; i < node->start + rnd_num; i ++ ) {
        cout << db->data[i][0] << " ";
    }
    cout << endl;
    {   // todo delete
        cout << "target_label:" << target_label << endl;
        cout << "Means:";
        for (const auto& mean : means) {
            cout << "\t(" << mean[0] << ")";
        }
        cout << endl;
    }


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

    cout << "clusters:" << cnt0 << " " << cnt1 << endl; // todo delete
    cout << "split_dis:" << split_dis << endl; // todo delete
    return split_dis;
}

void MixTreeKmeans::crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) {
    cout << "kmeans crack V" << endl;
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

    float split_dis = chose_split_dis(v_node);

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

    cout << "split num:" << cache_left.size() << " " << cache_right.size() << endl; // todo delete


    v_node->pivot = query;
    v_node->pivot_r = split_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

//void MixTreeKmeans::selectPivot(GNode *node, float* query) {
////    int rnd_num = min(20 * node->pivot_cnt, node->end - node->start + 1); // todo hard code
//    int rnd_num = node->end - node->start + 1; // todo hard code
//    vector<array<float, 100>> sample_rnd(rnd_num); // todo hard code
//    for (int i = 0; i < rnd_num; i ++ ) {
//        for (int j = 0; j < 100; j ++ ) { // todo hard code
//            sample_rnd[i][j] = db->data[node->start + i][j];
//        }
//    }
//    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd, node->pivot_cnt, max_iter); // k = pivot_cnt
//    const auto& means = std::get<0>(k_res);
//    const auto& labels = std::get<1>(k_res);
//
//    for (const auto & mean : means) {
//        auto* pivot = new float[db->dimension];
//        std::copy(mean.begin(), mean.end(), pivot);
//        node->pivots.emplace_back(pivot);
//    }
//    for (const auto& pivot : node->pivots) {
//        cout << "\t(" << pivot[0] << ")";
//    }
//    cout << endl;
//}
//
//void MixTreeKmeans::crackG(Node *node, Node* pre_node, float *query, float query_r, vector<float>& ans_dis) {
//    vector<float>().swap(node->cache_dis);
////    node->cache_dis.clear();
////    node->cache_dis.shrink_to_fit();
//
//    GNode* g_node;
//    if (node->type == NodeType::GNode) {
//        node->is_leaf = false;
//        g_node = (GNode*)node;
//    }
//    else {  // VNode to GNode
//        g_node = new GNode(node->start, node->end, max_pivot_cnt);
//        if (node != root) {
//            VNode* v_node = (VNode*) pre_node;
//            if (node == v_node->left_child) {
//                v_node->left_child = g_node;
//            }
//            else if (node == v_node->right_child) {
//                v_node->right_child = g_node;
//            }
//            else {
//                cout << "crack g error" << endl;
//                exit(255);
//            }
//            delete node;
//        }
//        else {
//            root = g_node;
//        }
//    }
//
//    int pivot_cnt = g_node->pivot_cnt;
//    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
//    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
//    selectPivot(g_node, query);
//    cout << "G " << pivot_cnt << endl;
//
//    vector<float> tmp_dis(pivot_cnt);
//    vector<vector<int>> pivot_data(pivot_cnt);  // 分配给各个支枢点的数据的id
//    vector<vector<float>> pivot_data_dist(pivot_cnt);    // 数据到分配的支枢点的距离
//    int data_cnt = g_node->end - g_node->start + 1;
//    for (int i = 0; i < data_cnt; i ++ ) {
//        float dis = calc_dis(db->dimension, db->data[g_node->start + i], query);
//        if (dis <= query_r) {
//            ans_dis.emplace_back(dis);
//        }
//
//        search_calc_cnt ++;
//        for (int j = 0; j < pivot_cnt; j ++ ) {
//            tmp_dis[j] = calc_dis(db->dimension, db->data[g_node->start + i], g_node->pivots[j]);   // 数据到第j个支枢点的距离
//        }
//        crack_calc_cnt += pivot_cnt;
//
//        int closest_pivot = (int)(min_element(tmp_dis.begin(), tmp_dis.end()) - tmp_dis.begin());
//        pivot_data[closest_pivot].emplace_back(g_node->start + i);
//        pivot_data_dist[closest_pivot].emplace_back(tmp_dis[closest_pivot]);
//        for (int j = 0; j < pivot_cnt; j ++ ) {
//            g_node->max_dis[j][closest_pivot] = max(g_node->max_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最远的obj的距离
//            g_node->min_dis[j][closest_pivot] = min(g_node->min_dis[j][closest_pivot], tmp_dis[j]);    // 各个pivot到closest_pivot中最近的obj的距离
//        }
//    }
//
//    float *data_tmp[data_cnt];
//    int tmp_len = 0;
//    for (int i = 0; i < pivot_cnt; i++) {
//        int new_node_start = tmp_len + g_node->start;
//        for (int data_id: pivot_data[i]) {
//            data_tmp[tmp_len] = db->data[data_id];
//            tmp_len++;
//        }
//        // create new node
//        int new_node_end = tmp_len + g_node->start - 1;
//        int next_pivot_cnt = (int)pivot_data[i].size() * avg_pivot_cnt * pivot_cnt / data_cnt;
//        next_pivot_cnt = max(min_pivot_cnt, next_pivot_cnt);
//        next_pivot_cnt = min(max_pivot_cnt, next_pivot_cnt);
//        GNode* leaf_node = new GNode(new_node_start, new_node_end, next_pivot_cnt, pivot_data_dist[i]);
//        g_node->children.emplace_back(leaf_node);
//    }
//    for (int i = g_node->start; i <= g_node->end; i ++ ) {
//        db->data[i] = data_tmp[i - g_node->start];
//    }
//
//    for (int i = 0; i < pivot_data.size(); i ++ ) {
//        cout << pivot_data[i].size() << " ";
//    }
//    cout << "| " << g_node->end - g_node->start + 1 << endl;
//}

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

    int pivot_cnt = g_node->pivot_cnt;
    g_node->min_dis.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    g_node->max_dis.resize(pivot_cnt, vector<float>(pivot_cnt, 0));
    selectPivot(g_node, query);
    cout << "G " << pivot_cnt << endl;


//    sort(db->data + g_node->start, db->data + g_node->end + 1, [&](const float* a, const float* b) {    // todo del
//        return a[0] < b[0];
//    });

    for (int i = g_node->start; i <= g_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += g_node->end - g_node->start + 1;

    float split_dis = chose_split_dis(g_node);

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

        pivot_data[closest_pivot].emplace_back(g_node->start + i);
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

    cout << "split num:";
    for (int i = 0; i < pivot_data.size(); i ++ ) {
        cout << pivot_data[i].size() << " ";
    }
    cout << "| " << g_node->end - g_node->start + 1 << endl;
}


//void MixTreeKmeans::rangeSearchImp(Node *node, Node* pre_node, float* query, float query_r, float pq_dis, vector<float> &ans_dis) {
//    if (node->is_leaf) {  // leaf node
//        if (node->end - node->start + 1 <= db->crack_threshold) {
//            search_start
//            if (node->type == NodeType::VNode) {
//                VNode* v_node = (VNode*) node;
//                for (int i = v_node->start; i <= v_node->end; i ++ ) {
//                    if (fabs(v_node->cache_dis[i - v_node->start] - pq_dis) > query_r) {  // todo 4 case(L2)
//                        continue;
//                    }
//                    query_dist[i] = calc_dis(db->dimension, db->data[i], query);
//                    search_calc_cnt ++;
//                    if (query_dist[i] <= query_r) {
//                        ans_dis.emplace_back(query_dist[i]);
//                    }
//                }
//            }
//            else {
//                GNode *g_node = (GNode *) node;
//                bool ok;
//                for (int i = g_node->start; i <= g_node->end; i ++ ) {
//                    ok = true;
//                    for (int j = 0; j < g_node->pivot_cnt; j ++ ) {
//                        if (fabs(g_node->cache_dis[i - node->start][j] - pq_dis) > query_r) {  // todo 4 case(L2)
//                            ok = false;
//                            break;
//                        }
//                    }
//                    if (!ok) continue;
//
//                    query_dist[i] = calc_dis(db->dimension, db->data[i], query);
//                    search_calc_cnt ++;
//                    if (query_dist[i] <= query_r) {
//                        ans_dis.emplace_back(query_dist[i]);
//                    }
//                }
//            }
//            search_end
//        }
//        else {
//            if (node->end - node->start + 1 <= db->tree_threshold) {
//                crack_start
//                cout << "G crack" << endl;
//                crackG(node, pre_node, query, query_r, ans_dis);
//                crack_end
//            }
//            else {
//                crack_start
//                cout << "V crack" << endl;
//                crackV(node, query, query_r, ans_dis);
//                crack_end
//            }
//        }
//    }
//    else {  // internal node
//        if (node->type == NodeType::VNode) {   // VNode
//            VNode* v_node = (VNode*) node;
//            float dis = calc_dis(db->dimension, v_node->pivot, query);
//            search_calc_cnt ++;
//            auto& left = v_node->left_child;
//            auto& right = v_node->right_child;
//
//            if (dis < query_r + v_node->pivot_r) {
//                if (dis < query_r - v_node->pivot_r) {  // query contains pivot area, data in left child is ans
//                    for (int i = left->start; i <= left->end; i ++ ) {
//                        ans_dis.emplace_back(calc_dis(db->dimension, query, db->data[i]));
//                    }
//                    search_calc_cnt += left->end - left->start;
//                    rangeSearchImp(right, node, query, query_r, dis, ans_dis);
//                }
//                else {
//                    rangeSearchImp(left, node, query, query_r, dis, ans_dis);
//                    if (dis >= v_node->pivot_r - query_r) { // pivots area doesn't contain query
//                        rangeSearchImp(right, node, query, query_r, dis, ans_dis);
//                    }
//                }
//            }
//            else {
//                rangeSearchImp(right, node, query, query_r, dis, ans_dis);
//            }
//
//        }
//        else if (node->type == NodeType::GNode){  // GNode
//            GNode* g_node = (GNode*) node;
//            size_t n = g_node->pivot_cnt;
//            float dis[n];
//            search_start
//            for (size_t i = 0; i < n; i ++ ) {
//                dis[i] = calc_dis(db->dimension, g_node->pivots[i], query);
//            }
//            search_calc_cnt += n;
//            search_end
//
////            float v_node_pivot_r = g_node->max_dis[0][0];
////            Node* left = g_node->children[0];
////            Node* right = g_node->children[1];
////            if (dis[0] < query_r + v_node_pivot_r) {
////                if (dis[0] < query_r - v_node_pivot_r) {  // query contains pivot area, data in left child is ans
////                    for (int i = left->start; i <= left->end; i ++ ) {
////                        ans_dis.emplace_back(calc_dis(db->dimension, query, db->data[i]));
////                    }
////                    search_calc_cnt += left->end - left->start;
////                    rangeSearchImp(right, node, query, query_r, dis[1], ans_dis);
////                }
////                else {
////                    rangeSearchImp(left, node, query, query_r, dis[0], ans_dis);
////                    if (dis[0] >= v_node_pivot_r - query_r) { // pivots area doesn't contain query
////                        rangeSearchImp(right, node, query, query_r, dis[1], ans_dis);
////                    }
////                }
////            }
////            else {
////                rangeSearchImp(right, node, query, query_r, dis[1], ans_dis);
////            }
//
//            for (size_t i = 0; i < n; i ++ ) {
//                bool flag = true;
//                for (size_t j = 0; flag && j < n; j ++ ) {
//                    flag &= (dis[j] <= g_node->max_dis[j][i] + query_r);
//                    flag &= (dis[j] >= g_node->min_dis[j][i] - query_r);
//                }
//
//                if (flag) {
//                    auto& child = g_node->children[i];
//                    rangeSearchImp(child, node, query, query_r, dis[i], ans_dis);
//                }
//            }
//        }
//        else {
//            cout << "node type not found" << endl;
//            exit(255);
//        }
//    }
//}

void MixTreeKmeans::knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) {
    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;
    vector<float>().swap(v_node->cache_dis);

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;


    int rnd_num = min(sample_num, v_node->end - v_node->start + 1);
    vector<array<float, 1>> sample_rnd_dis(rnd_num);
    for (int i = 0; i < rnd_num; i ++ ) {
        sample_rnd_dis[i][0] = query_dist[v_node->start + i];
    }
    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd_dis, 2, max_iter); // k = 2
    const auto& means = std::get<0>(k_res);
    const auto& labels = std::get<1>(k_res);
    int target_label = means[0][0] < means[1][0] ? 0 : 1;
    float med_dis = 0;
    for (int i = 0; i < labels.size(); i ++ ) {
        const auto& label = labels[i];
        if (label == target_label) {
            med_dis = max(med_dis, sample_rnd_dis[i][0]);
        }
    }

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(query_dist[l] <= med_dis && l <= v_node->end) {
            addAns(k, query_dist[l], db->data[l], ans_heap);
            l++;
        }
        while(query_dist[r] > med_dis && r >= v_node->start) {
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
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

