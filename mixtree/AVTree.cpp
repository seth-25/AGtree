#include <vector>
#include <cassert>
#include "MixTree.h"
#include "record.h"
#include "../dkm/dkm_parallel.hpp"
using namespace std;

void MixTree::crackV(Node *&node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    int rnd_dis[3];
    rnd_dis[0] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[1] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[2] = v_node->start + (rand() % (v_node->end - v_node->start + 1));

    float med_dis = max(min(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                        min(max(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                            distance[rnd_dis[2]])
                        );

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(distance[l] <= med_dis && l <= v_node->end) {
            if (distance[l] <= query_r) {
                ans_dis.emplace_back(distance[l]);
            }
            l++;
        }
        while(distance[r] > med_dis && r >= v_node->start) {
            if (distance[r] <= query_r) {
                ans_dis.emplace_back(distance[r]);
            }
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(distance[l], distance[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(distance.begin() + v_node->start, distance.begin() + r + 1, cache_left.begin());
    std::swap_ranges(distance.begin() + r + 1, distance.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTree::crackVSax(Node *&node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    float med_dis = 0;
    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    int node_num = v_node->end - v_node->start + 1;
    crack_calc_cnt += node_num;

    int rnd_dis[3];
    for (int i = 0; i < 3; i ++ ) {
        rnd_dis[i] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    }
    med_dis = max(min(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                        min(max(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                            distance[rnd_dis[2]])
    );

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(distance[l] <= med_dis && l <= v_node->end) {
            if (distance[l] <= query_r) {
                ans_dis.emplace_back(distance[l]);
            }
            l++;
        }
        while(distance[r] > med_dis && r >= v_node->start) {
            if (distance[r] <= query_r) {
                ans_dis.emplace_back(distance[r]);
            }
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(saxes[l], saxes[r]);
        swap(distance[l], distance[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(distance.begin() + v_node->start, distance.begin() + r + 1, cache_left.begin());
    std::swap_ranges(distance.begin() + r + 1, distance.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}


void MixTree::crackVKmeans(Node *&node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    int rnd_num = min(sample_num, v_node->end - v_node->start + 1);
    vector<array<float, 1>> sample_rnd_dis(rnd_num);
    for (int i = 0; i < rnd_num; i ++ ) {
        sample_rnd_dis[i][0] = distance[v_node->start + i];
    }
    auto k_res = dkm::kmeans_lloyd_parallel(sample_rnd_dis, 2, max_iter); // k = 2
    const auto& means = std::get<0>(k_res);
    const auto& labels = std::get<1>(k_res);
    int target_label = means[0][0] < means[1][0] ? 0 : 1;
//    cout << "target_label:" << target_label << endl;
//    cout << "Means:";
//    for (const auto& mean : means) {
//        cout << "\t(" << mean[0] << ")";
//    }
//    cout << endl;
    float med_dis = 0;
//    int cnt0 = 0, cnt1 = 0;
    for (int i = 0; i < labels.size(); i ++ ) {
        const auto& label = labels[i];
        if (label == target_label) {
            med_dis = max(med_dis, sample_rnd_dis[i][0]);
//            cnt0 ++;
        }
//        else {
//            cnt1 ++;
//        }
    }
//    cout << "clusters:" << cnt0 << " " << cnt1 << endl;
//    cout << "med_dis:" << med_dis << endl;


    int l = v_node->start, r = v_node->end;
    while(true) {
        while(distance[l] <= med_dis && l <= v_node->end) {
            if (distance[l] <= query_r) {
                ans_dis.emplace_back(distance[l]);
            }
            l++;
        }
        while(distance[r] > med_dis && r >= v_node->start) {
            if (distance[r] <= query_r) {
                ans_dis.emplace_back(distance[r]);
            }
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(distance[l], distance[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(distance.begin() + v_node->start, distance.begin() + r + 1, cache_left.begin());
    std::swap_ranges(distance.begin() + r + 1, distance.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}


void MixTree::knnCrackV(Node *node, float *query, int k, AnsHeap &ans_heap) {   // med cache
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;

    int rnd_dis[3];
    rnd_dis[0] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[1] = v_node->start + (rand() % (v_node->end - v_node->start + 1));
    rnd_dis[2] = v_node->start + (rand() % (v_node->end - v_node->start + 1));

    float med_dis = max(min(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                        min(max(distance[rnd_dis[0]], distance[rnd_dis[1]]),
                            distance[rnd_dis[2]])
    );

    int l = v_node->start, r = v_node->end;
    while(true) {
        while(distance[l] <= med_dis && l <= v_node->end) {
            addAns(k, distance[l], db->data[l], ans_heap);
            l++;
        }
        while(distance[r] > med_dis && r >= v_node->start) {
            addAns(k, distance[r], db->data[r], ans_heap);
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(distance[l], distance[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(distance.begin() + v_node->start, distance.begin() + r + 1, cache_left.begin());
    std::swap_ranges(distance.begin() + r + 1, distance.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}

void MixTree::knnCrackVKmeans(Node *node, float *query, int k, AnsHeap &ans_heap) {   // med cache
    vector<float>().swap(node->cache_dis);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = calc_dis(db->dimension, query, db->data[i]);
    }
    crack_calc_cnt += v_node->end - v_node->start + 1;


    int rnd_num = min(sample_num, v_node->end - v_node->start + 1);
    vector<array<float, 1>> sample_rnd_dis(rnd_num);
    for (int i = 0; i < rnd_num; i ++ ) {
        sample_rnd_dis[i][0] = distance[v_node->start + i];
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
        while(distance[l] <= med_dis && l <= v_node->end) {
            addAns(k, distance[l], db->data[l], ans_heap);
            l++;
        }
        while(distance[r] > med_dis && r >= v_node->start) {
            addAns(k, distance[r], db->data[r], ans_heap);
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(db->data[l], db->data[r]);
        swap(distance[l], distance[r]);
    }

    vector<float> cache_left(r - v_node->start + 1);
    vector<float> cache_right(v_node->end - r);
    std::swap_ranges(distance.begin() + v_node->start, distance.begin() + r + 1, cache_left.begin());
    std::swap_ranges(distance.begin() + r + 1, distance.begin() + v_node->end + 1, cache_right.begin());

    v_node->pivot = query;
    v_node->pivot_r = med_dis;
    v_node->left_child = new VNode(v_node->start, r, cache_left);
    v_node->right_child = new VNode(r + 1, v_node->end, cache_right);
}
