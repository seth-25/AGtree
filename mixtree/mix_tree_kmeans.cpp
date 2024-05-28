#include <queue>
#include "mix_tree_kmeans.h"
#include "record.h"
#include "../dkm/dkm_parallel.hpp"

using namespace std;

MixTreeKmeans::MixTreeKmeans(DB *db_): MixTreeCache(db_) {
    sample_num = 100;
    max_iter = 30;

    query_dist.reserve(db->num_data);
    vector<float> cache_dis(db->num_data, 0);
    root = new VNode(0, db->num_data - 1, cache_dis);
}

void MixTreeKmeans::crackV(Node *&node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

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
    cout << "target_label:" << target_label << endl;
    cout << "Means:";
    for (const auto& mean : means) {
        cout << "\t(" << mean[0] << ")";
    }
    cout << endl;
    float med_dis = 0;
    int cnt0 = 0, cnt1 = 0;
    for (int i = 0; i < labels.size(); i ++ ) {
        const auto& label = labels[i];
        if (label == target_label) {
            med_dis = max(med_dis, sample_rnd_dis[i][0]);
            cnt0 ++;
        }
        else {
            cnt1 ++;
        }
    }
    cout << "clusters:" << cnt0 << " " << cnt1 << endl;
    cout << "med_dis:" << med_dis << endl;



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

    cout << "split num:" << cache_left.size() << " " << cache_right.size() << endl;

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTreeKmeans::rangeSearchImp(Node *&node, float* query, float query_r, float pq_dis, vector<float> &ans_dis) {
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
                crackG(node, query, query_r, ans_dis);
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
                    rangeSearchImp(right, query, query_r, dis, ans_dis);
                }
                else {
                    rangeSearchImp(left, query, query_r, dis, ans_dis);
                    if (dis >= v_node->pivot_r - query_r) {
                        rangeSearchImp(right, query, query_r, dis, ans_dis);
                    }
                }
            }
            else {
                rangeSearchImp(right, query, query_r, dis, ans_dis);
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
                    rangeSearchImp(child, query, query_r, dis[i], ans_dis);
                }
            }
        }
        else {
            cout << "node type not found" << endl;
            exit(255);
        }
    }
}


void MixTreeKmeans::knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) {   // med cache
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

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


void MixTreeKmeans::knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) {
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
