#include <vector>
#include <cassert>
#include "MixTree.h"
#include "record.h"
using namespace std;

void MixTree::crackVCache(Node *&node, float *query, float query_r, std::vector<float> &ans_dis) {
    vector<float>().swap(node->cache_dist);

    node->is_leaf = false;
    assert(node->type == NodeType::VNode);
    VNode* v_node = (VNode*)node;

    for (int i = v_node->start; i <= v_node->end; i ++ ) {
        distance[i] = dist(db->dimension, query, db->data[i]);
    }
    crack_dis_cnt += v_node->end - v_node->start + 1;

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
    v_node->children.emplace_back(new VNode(v_node->start, r, cache_left));
    v_node->children.emplace_back(new VNode(r + 1, v_node->end, cache_right));
}