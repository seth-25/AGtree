#include <cfloat>
#include <algorithm>
#include <complex>
#include "AGtree.h"
#include "time_record.h"
using namespace std;

float l1_distance(int dim, const float *x, const float *y) {
    float ans = 0, res;
    for (int i = 0; i < dim; i++) {
        res = (x[i] - y[i]);
        if (res < 0) res = -res;
        ans += res;
    }
    return ans;
}

float l2_distance(int dim, const float *x, const float *y) {
    float ans = 0, res;
    int i;
    for (i = 0; i < dim; i++) {
        res = (x[i] - y[i]);
        ans += res * res;
    }
    return std::sqrt(ans);
}

#ifdef ED
#define dist(dim, data, query)
#endif
#ifdef L1
#define dist(dim, data, query) l1_distance(dim, data, query)
#endif
#ifdef L2
#define dist(dim, data, query) l2_distance(dim, data, query)
#endif

AGtree::AGtree(DB *db_) : db(db_) {
    avg_pivot_cnt = 5;
    max_pivot_cnt = min(4 * avg_pivot_cnt, 256);
    min_pivot_cnt = 2;

    pivot_dis.resize(3 * max_pivot_cnt, vector<float>(3 * max_pivot_cnt));
    pivot_pos.resize(max_pivot_cnt);

//    data_tmp = new float * [db->num_data];
//    for (int i = 0; i < db->num_data; i ++ ) {
//        data_tmp[i] = new float [db->dimension];
//    }

    root = new Node(0, db->num_data - 1, max_pivot_cnt);
}

void AGtree::crackInTwo(Node *node, float *query, float query_r, float *&dis, vector<float>& ans_dis) {
    node->pivot_cnt = 2;
    node->is_leaf = false;
    node->min_dist.resize(node->pivot_cnt, vector<float>(node->pivot_cnt, FLT_MAX));
    node->max_dist.resize(node->pivot_cnt, vector<float>(node->pivot_cnt, 0));

    float max_dis = -1;
    int pivot2_id;
    for (int i = node->start; i <= node->end; i ++ ) {
        if (dis[i] > max_dis) {
            max_dis = dis[i];
            pivot2_id = i;    // choose max distance data as pivot2
        }
    }
    float *pivot2 = db->data[pivot2_id];

    int i = node->start, j = node->end;
    float dis_i1, dis_i2, dis_j1, dis_j2;
    while(true) {
        while (i <= node->end) {
            dis_i1 = dis[i];  // pivot1
            dis_i2 = dist(db->dimension, db->data[i], pivot2);  // pivot2
            if (dis_i1 <= dis_i2) { // choose pivot1
                if (dis_i1 <= query_r) {
                    ans_dis.emplace_back(dis_i1);
                }
                i++;

                node->max_dist[0][0] = max(node->max_dist[0][0], dis_i1);
                node->max_dist[1][0] = max(node->max_dist[1][0], dis_i2);
                node->min_dist[0][0] = min(node->min_dist[0][0], dis_i1);
                node->min_dist[1][0] = min(node->min_dist[1][0], dis_i2);
            } else {
                break;
            }
        }

        while (j >= node->start) {
            dis_j1 = dis[j];  // pivot1
            dis_j2 = dist(db->dimension, db->data[j], pivot2);  // pivot2
            if (dis_j2 <= dis_j1) {  // choose pivot2
                if (dis_j1 <= query_r) {
                    ans_dis.emplace_back(dis_j1);
                }
                j--;

                node->max_dist[0][1] = max(node->max_dist[0][1], dis_j1);
                node->max_dist[1][1] = max(node->max_dist[1][1], dis_j2);
                node->min_dist[0][1] = min(node->min_dist[0][1], dis_j1);
                node->min_dist[1][1] = min(node->min_dist[1][1], dis_j2);
            } else {
                break;
            }
        }

        if (i >= j) break;
        swap(db->data[i], db->data[j]);
        swap(dis[i], dis[j]);
    }


    node->pivots.emplace_back(query);
    node->children.emplace_back(new Node(node->start, j, 2));

    node->pivots.emplace_back(pivot2);
    node->children.emplace_back(new Node(j + 1, node->end, 2));
}

void AGtree::selectPivot(Node *node, float* query) {
    //calc dist between samples
    int sample_cnt = min(node->pivot_cnt * 3, node->end - node->start + 2); // data + query
    for (int i = 0; i < sample_cnt - 1; i ++ ) {
        for (int j = 0; j < sample_cnt - 1; j ++ ) {
            pivot_dis[i][j] = pivot_dis[j][i] = dist(db->dimension, db->data[node->start + i], db->data[node->start + j]);
        }
    }
    pivot_dis[sample_cnt - 1][sample_cnt - 1] = 0;
    for (int i = 0; i < sample_cnt - 1; i ++ ) {      // calc dist between data and query
        pivot_dis[i][sample_cnt - 1] = pivot_dis[sample_cnt - 1][i] = dist(db->dimension, db->data[node->start + i], query);
    }

    //select query as first pivot
    vector<bool> is_pivot(sample_cnt, false);
    int p = sample_cnt - 1;
    pivot_pos[0] = p;
    is_pivot[p] = true;

    // select pivots
    vector<float> min_dis(sample_cnt, FLT_MAX);
    for (int i = 1; i < node->pivot_cnt; i ++ ) {
        for (int j = 0; j < sample_cnt; j ++ ) {
            min_dis[j] = min(min_dis[j], pivot_dis[j][pivot_pos[i - 1]]);   // min dist between j and pivots which has selected
        }
        for (p = 0; is_pivot[p]; p ++ );    // initialize pivot p
        for (int j = p + 1; j < sample_cnt; j ++ ) {
            if (min_dis[j] > min_dis[p] && !is_pivot[j]) {  // point that has max value of min dist is the p
                p = j;
            }
        }
        pivot_pos[i] = p;
        is_pivot[p] = true;
    }
    node->pivots.emplace_back(query);
    for (int i = 1; i < node->pivot_cnt; i ++ ) {
        node->pivots.emplace_back(db->data[node->start + pivot_pos[i]]);
    }
}

void AGtree::crackInMany(Node *node, float *query, float query_r, float *&dis, vector<float>& ans_dis) {
    int pivot_cnt = node->pivot_cnt;
    node->is_leaf = false;
    node->min_dist.resize(pivot_cnt, vector<float>(pivot_cnt, FLT_MAX));
    node->max_dist.resize(pivot_cnt, vector<float>(pivot_cnt, 0));

    selectPivot(node, query);

    vector<float> data_dis;
    vector<vector<int>> pivot_data(pivot_cnt);
    int data_cnt = node->end - node->start + 1;
    for (int i = 0; i < data_cnt; i ++ ) {
        data_dis.resize(pivot_cnt, FLT_MAX);
        for (int j = 0; j < pivot_cnt; j ++ ) {
            data_dis[j] = dist(db->dimension, db->data[node->start + i], node->pivots[j]);
            if (j == 0) {   // pivot is query
                if (data_dis[j] < query_r) {
                    ans_dis.emplace_back(data_dis[j]);
                }
            }
        }
        int closest_pivot = (int)(min_element(data_dis.begin(), data_dis.end()) - data_dis.begin());
        pivot_data[closest_pivot].emplace_back(node->start + i);
        for (int j = 0; j < pivot_cnt; j ++ ) {
            node->max_dist[j][closest_pivot] = max(node->max_dist[j][closest_pivot], data_dis[j]);    // 各个pivot到closest_pivot中最远的obj的距离
            node->min_dist[j][closest_pivot] = min(node->min_dist[j][closest_pivot], data_dis[j]);    // 各个pivot到closest_pivot中最近的obj的距离
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
        int new_node_end = tmp_len + node->start - 1;
        int next_pivot_cnt = (int)pivot_data[i].size() * avg_pivot_cnt * pivot_cnt / data_cnt;
        next_pivot_cnt = max(min_pivot_cnt, next_pivot_cnt);
        next_pivot_cnt = min(max_pivot_cnt, next_pivot_cnt);
        node->children.emplace_back(new Node(new_node_start, new_node_end, next_pivot_cnt));
    }
    for (int i = node->start; i <= node->end; i ++ ) {
        db->data[i] = data_tmp[i - node->start];
    }
}


void AGtree::search(Node *node, float* query, float query_r, int query_id, float *&distance, vector<float>& ans_dis) {
    if (!node->is_leaf) {
        size_t n = node->pivot_cnt;
        float dis[n];
        search_start
        for (size_t i = 0; i < n; i ++ ) {
            dis[i] = dist(db->dimension, node->pivots[i], query);
        }
        search_end
        for (size_t i = 0; i < n; i ++ ) {
            bool flag = true;
            for (size_t j = 0; flag && j < n; j ++ ) {
                flag &= (node->max_dist[j][i] >= dis[j] - query_r);
                flag &= (node->min_dist[j][i] <= dis[j] + query_r);
            }
            if (flag) {
                auto& child = node->children[i];
                search(child, query, query_r, query_id, distance, ans_dis);
            }
        }
    }
    else {
        search_start
        for (int i = node->start; i <= node->end; i ++ ) {
            distance[i] = dist(db->dimension, db->data[i], query);
        }
        search_end
        cnt_calc_dis += node->end - node->start + 1;
        if (node->end - node->start + 1 <= db->crack_threshold) {
            search_start
            for (int i = node->start; i <= node->end; i ++ ) {
                if (distance[i] <= query_r) {
                    ans_dis.emplace_back(distance[i]);
                }
            }
            search_end
        }
        else {
            crack_start
//            crackInTwo(node, query, query_r, distance, ans_dis);
            crackInMany(node, query, query_r, distance, ans_dis);
            crack_end
        }
    }
}

//void AGtree::searchCache(Node *node, float* query, float query_r, int query_id, float *&distance, vector<float>& ans_dis) {
//    if (!node->is_leaf) {
//        size_t n = node->pivot_cnt;
//        float dis[n];
//        search_start
//        for (size_t i = 0; i < n; i ++ ) {
//            dis[i] = dist(db->dimension, node->pivots[i], query);
//        }
//        search_end
//        for (size_t i = 0; i < n; i ++ ) {
//            bool flag = true;
//            for (size_t j = 0; flag && j < n; j ++ ) {
//                flag &= (node->max_dist[j][i] >= dis[j] - query_r);
//                flag &= (node->min_dist[j][i] <= dis[j] + query_r);
//            }
//            if (flag) {
//                auto& child = node->children[i];
//                search(child, query, query_r, query_id, distance, ans_dis);
//            }
//        }
//    }
//    else {
//        search_start
//        for (int i = node->start; i <= node->end; i ++ ) {
//            distance[i] = dist(db->dimension, db->data[i], query);
//        }
//        search_end
//        cnt_calc_dis += node->end - node->start + 1;
//        if (node->end - node->start + 1 <= db->crack_threshold) {
//            search_start
//            for (int i = node->start; i <= node->end; i ++ ) {
//                if (distance[i] <= query_r) {
//                    ans_dis.emplace_back(distance[i]);
//                }
//            }
//            search_end
//        }
//        else {
//            crack_start
////            crackInTwo(node, query, query_r, distance, ans_dis);
//            crackInMany(node, query, query_r, distance, ans_dis);
//            crack_end
//        }
//    }
//}
