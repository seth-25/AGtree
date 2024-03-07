#include <cmath>
#include <complex>
#include <algorithm>
#include <queue>
#include "MixTree.h"
#include "Node.h"
#include "record.h"

using namespace std;


MixTree::MixTree(DB *db_): db(db_) {
    avg_pivot_cnt = 5;
    max_pivot_cnt = min(2 * avg_pivot_cnt, 256);
    min_pivot_cnt = 2;
    distance.reserve(db->num_data);
    pivot_dis.resize(3 * max_pivot_cnt, vector<float>(3 * max_pivot_cnt));
    pivot_pos.reserve(max_pivot_cnt);

    vector<float> cache_dis(db->num_data, 0);
    root = new VNode(0, db->num_data - 1, cache_dis);
//    root = new GNode(0, db->num_data - 1, max_pivot_cnt, cache_dis);
}

void MixTree::rangeSearch(float *query, float query_r, std::vector<float> &ans_dis) {
    switch (db->method) {
        case Method::STANDARD: {

        } break;
        case Method::CACHE: {
            rangeSearchCache(root, query, query_r, 0, ans_dis);
        } break;
        default: {
            cout << "method not found" << endl;
        }
    }
}

void MixTree::knnSearch(float *query, int k, AnsHeap &ans_dis) {
    typedef tuple<float, Node *, int> knn_ext_pair;
    priority_queue<knn_ext_pair,vector<knn_ext_pair>,greater<knn_ext_pair>> guide_pq;
    priority_queue<knn_ext_pair> result;

    NodeHeap node_heap;
    node_heap.emplace(0, root, nullptr);
    switch (db->method) {
        case Method::STANDARD: {

        } break;
        case Method::CACHE: {
            knnSearchCache(query, k, node_heap, ans_dis);
        } break;
        default: {
            cout << "method not found" << endl;
        }
    }
}

void MixTree::rangeSearchCache(Node *&node, float* query, float query_r, float pq_dis, vector<float> &ans_dis) {
    if (node->is_leaf) {  // leaf node
        if (node->end - node->start + 1 <= db->crack_threshold) {
            search_start
            for (int i = node->start; i <= node->end; i ++ ) {
                if (fabs(node->cache_dis[i - node->start] - pq_dis) > query_r) {  // todo 4 case(L2)
                    continue;
                }
                distance[i] = calc_dis(db->dimension, db->data[i], query);
                search_calc_cnt ++;
                if (distance[i] <= query_r) {
                    ans_dis.emplace_back(distance[i]);
                }
            }
            search_end
        }
        else {
            if (node->end - node->start + 1 <= db->tree_threshold) {
                crack_start
                cout << "G crack" << endl;
                crackGCache(node, query, query_r, ans_dis);
                crack_end
            }
            else {
                crack_start
                cout << "V crack" << endl;
                crackVCache(node, query, query_r, ans_dis);
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
                    rangeSearchCache(right, query, query_r, dis, ans_dis);
                }
                else {
                    rangeSearchCache(left, query, query_r, dis, ans_dis);
                    if (dis >= v_node->pivot_r - query_r) {
                        rangeSearchCache(right, query, query_r, dis, ans_dis);
                    }
                }
            }
            else {
                rangeSearchCache(right, query, query_r, dis, ans_dis);
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
                    rangeSearchCache(child, query, query_r, dis[i], ans_dis);
                }
            }
        }
        else {
            cout << "node type not found" << endl;
            exit(255);
        }
    }
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

void MixTree::knnSearchCache(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) {
    while(!node_heap.empty() && (ans_heap.size() < k || get<0>(node_heap.top()) < ans_heap.top().first)) {
        NodeTuple node_tuple = node_heap.top();
        Node* node = get<1>(node_tuple);
        if (node->is_leaf) {
            if (node->end - node->start + 1 <= db->crack_threshold) {
                for (int i = node->start; i <= node->end; i ++ ) {
                    distance[i] = calc_dis(db->dimension, query, db->data[i]);
                    addAns(k, distance[i], db->data[i], ans_heap);
                }
                search_calc_cnt += node->end - node->start + 1;
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
                    node_heap.emplace(left_min_dis, left, v_node);
                }
                if (ans_heap.size() < k || right_min_dis < ans_heap.top().first) {
                    Node*& right = v_node->right_child;
                    node_heap.emplace(right_min_dis, right, v_node);
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
//                        cout << "min_dis:" << min_dis << endl;
                        final_min_dis = max(min_dis, final_min_dis);
                        flag &= (ans_heap.size() < k || min_dis < ans_heap.top().first);
                    }
//                    cout << final_min_dis << endl;
                    if (flag) {
                        auto& child = g_node->children[i];
                        node_heap.emplace(final_min_dis, child, node);
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