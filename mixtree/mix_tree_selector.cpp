#include <queue>
#include <cfloat>
#include "record.h"
#include "../dkm/dkm_parallel.hpp"
#include "mix_tree_selector.h"

using namespace std;

MixTreeSelector::MixTreeSelector(DB *db_): MixTreeKmeans(db_) {
    int first_sample_num = 2000;
    const int dim = 100;
    const int first_k = 10;
    // 采样1000个db->data，对db->data所有维度进行kmeans聚类，得到10个簇
    vector<array<float, dim>> sample_rnd(first_sample_num); // todo hard code
    for (int i = 0; i < first_sample_num; i ++ ) {
        for (int j = 0; j < dim; j ++ ) { // todo hard code
            sample_rnd[i][j] = db->data[i][j];
        }
    }
    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd, first_k, max_iter); // k = 100
    const auto& means = std::get<0>(k_res);
    const auto& labels = std::get<1>(k_res);
    for (const auto & mean : means) {
        auto* pivot = new float[db->dimension];
        std::copy(mean.begin(), mean.end(), pivot);
        selector.emplace_back(pivot, 0, 0);
    }
}

void MixTreeSelector::checkCrack(float *query) {
    need_crack = false;
    if (selector.empty()) {
        return;
    }
    float min_dis = FLT_MAX;
    int min_idx = 0;
    for (int i = 0; i < selector.size(); i ++ ) {
        float dis = calc_dis(db->dimension, query, selector[i].pivot);
        if (dis < min_dis) {
            min_dis = dis;
            min_idx = i;
        }
    }

    selector[min_idx].cnt ++;
    if (selector[min_idx].cnt >= fibonacci[selector[min_idx].id]) {
        selector[min_idx].cnt = 0;
        selector[min_idx].id ++;
        need_crack = true;
    }
}

void MixTreeSelector::rangeSearchImpRec(Node *node, Node *pre_node, float *query, float query_r, std::vector<float> pq_dis,
                                     std::vector<float> &ans_dis) {
    if (node->is_leaf) {  // leaf node
        if (!need_crack || node->end - node->start + 1 <= db->crack_threshold) {
            search_start
            if (node->type == NodeType::VNode) {
                VNode* v_node = (VNode*) node;
                for (int i = v_node->start; i <= v_node->end; i ++ ) {
                    if (fabs(v_node->cache_dis[i - v_node->start] - pq_dis[0]) > query_r) {  // todo 4 case(L2)
                        continue;
                    }
                    query_dist[i] = calc_dis(db->dimension, db->data[i], query);
                    search_calc_cnt ++;
                    if (query_dist[i] <= query_r) {
                        ans_dis.emplace_back(query_dist[i]);
                    }
                }
            }
            else {
                GNode *g_node = (GNode *) node;
                bool ok;
                for (int i = g_node->start; i <= g_node->end; i ++ ) {
                    ok = true;
                    for (int j = 0; j < g_node->cache_dis[0].size(); j ++ ) {
                        if (fabs(g_node->cache_dis[i - node->start][j] - pq_dis[j]) > query_r) {  // todo 4 case(L2)
                            ok = false;
                            break;
                        }
                    }
                    if (!ok) continue;

                    query_dist[i] = calc_dis(db->dimension, db->data[i], query);
                    search_calc_cnt ++;
                    if (query_dist[i] <= query_r) {
                        ans_dis.emplace_back(query_dist[i]);
                    }
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
            vector<float> dis(1);
            dis[0] = calc_dis(db->dimension, v_node->pivot, query);
            search_calc_cnt ++;
            auto& left = v_node->left_child;
            auto& right = v_node->right_child;
            if (dis[0] <= query_r + v_node->pivot_r) {
                if (dis[0] <= query_r - v_node->pivot_r) {  // all data in left child is ans
                    for (int i = left->start; i <= left->end; i ++ ) {
                        ans_dis.emplace_back(calc_dis(db->dimension, query, db->data[i]));
                    }
                    search_calc_cnt += left->end - left->start;
                    rangeSearchImpRec(right, node, query, query_r, dis, ans_dis);
                }
                else {
                    rangeSearchImpRec(left, node, query, query_r, dis, ans_dis);
                    if (dis[0] > v_node->pivot_r - query_r) {
                        rangeSearchImpRec(right, node, query, query_r, dis, ans_dis);
                    }
                }
            }
            else {
                rangeSearchImpRec(right, node, query, query_r, dis, ans_dis);
            }

        }
        else if (node->type == NodeType::GNode){  // GNode
            GNode* g_node = (GNode*) node;
            size_t n = g_node->pivot_cnt;
            vector<float> dis(n);
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
                    rangeSearchImpRec(child, node, query, query_r, dis, ans_dis);
                }
            }
        }
        else {
            cout << "node type not found" << endl;
            exit(255);
        }
    }
}

void MixTreeSelector::rangeSearchImp(Node *node, Node* pre_node, float* query, float query_r, vector<float> pq_dis, vector<float> &ans_dis) {
    checkCrack(query);

    cout << "selector " << endl;
    for(int i = 0; i < selector.size(); i ++ ) {
        cout << "(" << selector[i].cnt << ", " << selector[i].id << ") ";
    }
    cout << endl << "need crack:" << need_crack << endl;

//    need_crack = true;
    rangeSearchImpRec(node, pre_node, query, query_r, pq_dis, ans_dis);
}
