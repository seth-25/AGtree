#include <cmath>
#include <complex>
#include <algorithm>
#include "MixTree.h"
#include "Node.h"
#include "record.h"

using namespace std;


MixTree::MixTree(DB *db_): db(db_) {
    avg_pivot_cnt = 3;
    max_pivot_cnt = min(2 * avg_pivot_cnt, 256);
    min_pivot_cnt = 2;
    distance.reserve(db->num_data);
    pivot_dis.resize(3 * max_pivot_cnt, vector<float>(3 * max_pivot_cnt));
    pivot_pos.reserve(max_pivot_cnt);

    vector<float> cache_dist(db->num_data, 0);
    root = new VNode(0, db->num_data - 1, cache_dist);
//    root = new GNode(0, db->num_data - 1, max_pivot_cnt, cache_dist);
}

void MixTree::search(float *query, float query_r, std::vector<float> &ans_dis) {
    switch (db->method) {
        case Method::STANDARD: {

        } break;
        case Method::CACHE: {
            searchCache(root, query, query_r, 0, ans_dis);
        } break;
        default: {
            cout << "method not found" << endl;
        }

    }
}

void MixTree::searchCache(Node *&node, float* query, float query_r, float pq_dist, vector<float>& ans_dis) {
    if (node->is_leaf) {  // leaf node
        if (node->end - node->start + 1 <= db->crack_threshold) {
            search_start
            for (int i = node->start; i <= node->end; i ++ ) {
                if (fabs(node->cache_dist[i - node->start] - pq_dist) > query_r) {
                    continue;
                }
                distance[i] = dist(db->dimension, db->data[i], query);
                search_dis_cnt ++;
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
                crackGInManyCache(node, query, query_r, ans_dis);
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
            float dis = dist(db->dimension, v_node->pivot, query);
            search_dis_cnt ++;
            auto& left = v_node->children[0];
            auto& right = v_node->children[1];
            if (dis < query_r + v_node->pivot_r) {
                if (dis < query_r - v_node->pivot_r) {  // all data in left child is ans
                    for (int i = left->start; i <= left->end; i ++ ) {
                        float dis = dist(db->dimension, query, db->data[i]);
                        ans_dis.emplace_back(dis);
                    }
                    search_dis_cnt += left->end - left->start;
                    searchCache(right, query, query_r, dis, ans_dis);
                }
                else {
                    searchCache(left, query, query_r, dis, ans_dis);
                    if (dis >= v_node->pivot_r - query_r) {
                        searchCache(right, query, query_r, dis, ans_dis);
                    }
                }
            }
            else {
                searchCache(right, query, query_r, dis, ans_dis);
            }

        }
        else if (node->type == NodeType::GNode){  // GNode
            GNode* g_node = (GNode*) node;
            size_t n = g_node->pivot_cnt;
            float dis[n];
            search_start
            for (size_t i = 0; i < n; i ++ ) {
                dis[i] = dist(db->dimension, g_node->pivots[i], query);
            }
            search_dis_cnt += n;
            search_end
            for (size_t i = 0; i < n; i ++ ) {
                bool flag = true;
                for (size_t j = 0; flag && j < n; j ++ ) {
                    flag &= (g_node->max_dist[j][i] >= dis[j] - query_r);
                    flag &= (g_node->min_dist[j][i] <= dis[j] + query_r);
                }
                if (flag) {
                    auto& child = g_node->children[i];
                    searchCache(child, query, query_r, dis[i], ans_dis);
                }
            }
        }
        else {
            cout << "node type not found" << endl;
            exit(255);
        }
    }
}