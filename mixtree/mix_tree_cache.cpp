#include "mix_tree_cache.h"
#include <cmath>
#include <complex>
#include <algorithm>
#include <cfloat>
#include "record.h"

using namespace std;

MixTreeCache::MixTreeCache(DB *db_): MixTree(db_) {
    query_dist.resize(db->num_data);
    vector<float> cache_dis(db->num_data, 0);
    root = new VNode(0, db->num_data - 1, cache_dis);
}


MixTreeCache::~MixTreeCache() {
    delete root;
}

void MixTreeCache::crackV(Node *node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    int rnd_dis[3];
    rnd_dis[0] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[1] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[2] = v_node->start + (rand() % (v_node->end - v_node->start + 1));

    float med_dis = max(min(query_dist[rnd_dis[0]], query_dist[rnd_dis[1]]),
                        min(max(query_dist[rnd_dis[0]], query_dist[rnd_dis[1]]),
                            query_dist[rnd_dis[2]])
    );

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(query_dist[l] <= med_dis && l <= v_node->end) {
            if (query_dist[l] <= query_r) {
                ans_dis.emplace_back(query_dist[l]);
            }
            l++;
        }
        while(query_dist[r] > med_dis && r >= v_node->start) {
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

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(query_dist.begin() + v_node->start, query_dist.begin() + r + 1, cache_left.begin());
    std::swap_ranges(query_dist.begin() + r + 1, query_dist.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTreeCache::crackG(Node *node, Node* pre_node, float *query, float query_r, vector<float>& ans_dis) {
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
                if (tmp_dis[j] <= query_r) {
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



void MixTreeCache::rangeSearchImp(Node *node, Node* pre_node, float* query, float query_r, float pq_dis, vector<float> &ans_dis) {
    if (node->is_leaf) {  // leaf node
        if (node->end - node->start + 1 <= db->crack_threshold) {
            search_start
            for (int i = node->start; i <= node->end; i ++ ) {
                if (fabs(node->cache_dis[i - node->start] - pq_dis) > query_r) {  // todo 4 case(L2)
                    continue;
                }
                query_dist[i] = calc_dis(db->dimension, db->data[i], query);
                search_calc_cnt ++;
                if (query_dist[i] <= query_r) {
                    ans_dis.emplace_back(query_dist[i]);
                }
            }
            search_end
        }
        else {
            if (node->end - node->start + 1 <= db->tree_threshold) {
                crack_start
                cout << "G crack" << endl;
                crackG(node, pre_node, query, query_r, ans_dis);
                crack_end
            }
            else {
                crack_start
                cout << "V crack" << endl;
                crackV(node, query, query_r, ans_dis);
                crack_end
            }
        }
    }
    else {  // internal node
        if (node->type == NodeType::VNode) {   // VNode
            VNode* v_node = (VNode*) node;
            float dis = calc_dis(db->dimension, v_node->pivot, query);
            search_calc_cnt ++;
            auto& left = v_node->left_child;
            auto& right = v_node->right_child;
            if (dis < query_r + v_node->pivot_r) {
                if (dis < query_r - v_node->pivot_r) {  // all data in left child is ans
                    for (int i = left->start; i <= left->end; i ++ ) {
                        ans_dis.emplace_back(calc_dis(db->dimension, query, db->data[i]));
                    }
                    search_calc_cnt += left->end - left->start;
                    rangeSearchImp(right, node, query, query_r, dis, ans_dis);
                }
                else {
                    rangeSearchImp(left, node, query, query_r, dis, ans_dis);
                    if (dis >= v_node->pivot_r - query_r) {
                        rangeSearchImp(right, node, query, query_r, dis, ans_dis);
                    }
                }
            }
            else {
                rangeSearchImp(right, node, query, query_r, dis, ans_dis);
            }

        }
        else if (node->type == NodeType::GNode){  // GNode
            GNode* g_node = (GNode*) node;
            size_t n = g_node->pivot_cnt;
            float dis[n];
            search_start
            for (size_t i = 0; i < n; i ++ ) {
                dis[i] = calc_dis(db->dimension, g_node->pivots[i], query);
            }
            search_calc_cnt += n;
            search_end
            for (size_t i = 0; i < n; i ++ ) {
                bool flag = true;
                for (size_t j = 0; flag && j < n; j ++ ) {
                    flag &= (g_node->max_dis[j][i] >= dis[j] - query_r);
                    flag &= (g_node->min_dis[j][i] <= dis[j] + query_r);
                }
                if (flag) {
                    auto& child = g_node->children[i];
                    rangeSearchImp(child, node, query, query_r, dis[i], ans_dis);
                }
            }
        }
        else {
            cout << "node type not found" << endl;
            exit(255);
        }
    }
}





void MixTreeCache::knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) {   // med cache
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    int rnd_dis[3];
    rnd_dis[0] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[1] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[2] = v_node->start + (rand() % (v_node->end - v_node->start + 1));

    float med_dis = max(min(query_dist[rnd_dis[0]], query_dist[rnd_dis[1]]),
                        min(max(query_dist[rnd_dis[0]], query_dist[rnd_dis[1]]),
                            query_dist[rnd_dis[2]])
    );

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


void MixTreeCache::knnCrackG(Node *node, Node* pre_node, float *query, int k, AnsHeap &ans_dis) {
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


int cnt = 0;

void MixTreeCache::knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) {
    cnt ++; // todo delete
    while(!node_heap.empty() && (ans_heap.size() < k || get<0>(node_heap.top()) < ans_heap.top().first)) {
        NodeTuple node_tuple = node_heap.top();
        Node* node = get<1>(node_tuple);
        float pq_dis = get<3>(node_tuple);
        if (node->is_leaf) {
            if (node->end - node->start + 1 <= db->crack_threshold) {
                for (int i = node->start; i <= node->end; i ++ ) {
                    if (ans_heap.size() >= k && fabs(node->cache_dis[i - node->start] - pq_dis) > ans_heap.top().first) {  // todo 4 case(L2)
                        continue;
                    }
                    query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
//                    if (cnt == 998 && !ans_heap.empty())
//                        cout << fabs(node->cache_dis[i - node->start] - pq_dis) << " " << query_dist[i] << " " <<  ans_heap.top().first << " " << ans_heap.size() << endl;
                    search_calc_cnt ++;
                    addAns(k, query_dist[i], db->data[i], ans_heap);
                }
            }
            else {
                if (node->end - node->start + 1 <= db->tree_threshold) {
                    crack_start
                    cout << "G crack" << endl;
                    knnCrackG(node, get<2>(node_tuple), query, k, ans_heap);
                    crack_end
                }
                else {
                    crack_start
                    cout << "V crack" << endl;
                    knnCrackV(node, query, k, ans_heap);
                    crack_end
                }
            }
            node_heap.pop();
        }
        else {
            if (node->type == NodeType::VNode) {
//                cout << "V node" << endl;
                VNode* v_node = (VNode*) node;
                node_heap.pop();
                float dis = calc_dis(db->dimension, v_node->pivot, query);
                search_calc_cnt ++;
                float left_min_dis = max(0.0f, dis - v_node->pivot_r);
                float right_min_dis = max(0.0f, v_node->pivot_r - dis);
                if (ans_heap.size() < k || left_min_dis < ans_heap.top().first) {
                    Node*& left = v_node->left_child;
                    node_heap.emplace(left_min_dis, left, v_node, dis);
                }
                if (ans_heap.size() < k || right_min_dis < ans_heap.top().first) {
                    Node*& right = v_node->right_child;
                    node_heap.emplace(right_min_dis, right, v_node, dis);
                }
            }
            else if (node->type == NodeType::GNode) {
//                cout << "G node" << endl;
                GNode* g_node = (GNode*) node;
                node_heap.pop();
                size_t n = g_node->pivot_cnt;
                float dis[n];
                search_start
                for (size_t i = 0; i < n; i ++ ) {
                    dis[i] = calc_dis(db->dimension, g_node->pivots[i], query);
                }
                search_calc_cnt += n;
                search_end
                float min_dis, final_min_dis;
                for (size_t i = 0; i < n; i ++ ) {
                    bool flag = true;
                    final_min_dis = 0;
                    for (size_t j = 0; flag && j < n; j ++ ) {
                        min_dis = max(0.0f, max(dis[j] - g_node->max_dis[j][i], g_node->min_dis[j][i] - dis[j]));
                        final_min_dis = max(min_dis, final_min_dis);
                        flag &= (ans_heap.size() < k || min_dis < ans_heap.top().first);
                    }
                    if (flag) {
                        auto& child = g_node->children[i];
                        node_heap.emplace(final_min_dis, child, node, dis[i]);
                    }
                }
            }
            else {
                cout << "node type not found" << endl;
                exit(255);
            }
        }
    }
}
