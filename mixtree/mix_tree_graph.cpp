#include "mix_tree_graph.h"
#include "mix_tree_kmeans.h"
#include "record.h"

using namespace std;

MixTreeGraph::MixTreeGraph(DB *db_) : MixTreeKmeans(db_) {
    M = 64;
    efConstruction = 256;

    l2space = new hnswlib::L2Space(db->dimension);
    alg_hnsw = new hnswlib::HierarchicalNSW<float>(l2space, db->num_data, M, efConstruction);  // todo N = db->num_data 可以变小
}


MixTreeGraph::~MixTreeGraph() {
    delete alg_hnsw;
    delete l2space;
}

void MixTreeGraph::addGraph(Node* node) {
    int node_id = graph_nodes.size();
    graph_nodes.emplace_back(node);
    if (node->type == NodeType::VNode) {
        VNode* vnode = (VNode*) node;
        alg_hnsw->addPoint(vnode->father_pivot, node_id);
        cout << "add graph vnode" << endl;
        cout << endl;
        vnode->in_graph = true;
    }
    else {
        GNode* gnode = (GNode*) node;
        alg_hnsw->addPoint(gnode->father_pivot, node_id);
        cout << "add graph gnode" << endl;
        gnode->in_graph = true;
    }
}

void MixTreeGraph::exactKnnSearch(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap, vector<int>& node_ids) {
    while(!node_heap.empty() && (ans_heap.size() < k || get<0>(node_heap.top()) < ans_heap.top().first)) {
        NodeTuple node_tuple = node_heap.top();
        Node* node = get<1>(node_tuple);
        vector<float>& pq_dis = get<3>(node_tuple);
        if (node->is_leaf) {
            if (node->end - node->start + 1 <= db->crack_threshold) {
                if (node->type == NodeType::VNode) {
                    VNode* v_node = (VNode*) node;
                    if (v_node->has_search) {
                        node_heap.pop();
                        continue;
                    }
                    for (int i = v_node->start; i <= v_node->end; i ++ ) {
                        if (ans_heap.size() >= k && fabs(v_node->cache_dis[i - v_node->start] - pq_dis[0]) > ans_heap.top().first) {  // todo 4 case(L2)
                            continue;
                        }
                        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
                        search_calc_cnt ++;
                        addAns(k, query_dist[i], db->data[i], ans_heap);
                    }
                    if (!v_node->in_graph) {
                        addGraph(node);
                    }
                }
                else {
                    GNode* g_node = (GNode*) node;
                    if (g_node->has_search) {
                        node_heap.pop();
                        continue;
                    }
                    bool ok;
                    for (int i = g_node->start; i <= g_node->end; i ++ ) {
                        ok = true;
                        for (int j = 0; j < g_node->pivot_cnt; j ++ ) {
                            if (ans_heap.size() >= k && fabs(g_node->cache_dis[i - g_node->start][j] - pq_dis[j]) > ans_heap.top().first) {  // todo 4 case(L2)
                                ok = false;
                                break;
                            }
                        }

                        if (!ok) continue;
                        query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
                        search_calc_cnt ++;
                        addAns(k, query_dist[i], db->data[i], ans_heap);
                    }
                    if (!g_node->in_graph) {
                        addGraph(node);
                    }
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
                vector<float> new_dis(1, dis);
                search_calc_cnt ++;
                float left_min_dis = max(0.0f, dis - v_node->pivot_r);
                float right_min_dis = max(0.0f, v_node->pivot_r - dis);
                if (ans_heap.size() < k || left_min_dis < ans_heap.top().first) {
                    Node*& left = v_node->left_child;
                    node_heap.emplace(left_min_dis, left, v_node, new_dis);
                }
                if (ans_heap.size() < k || right_min_dis < ans_heap.top().first) {
                    Node*& right = v_node->right_child;
                    node_heap.emplace(right_min_dis, right, v_node, new_dis);
                }
            }
            else if (node->type == NodeType::GNode) {
//                cout << "G node" << endl;
                GNode* g_node = (GNode*) node;
                node_heap.pop();
                size_t n = g_node->pivot_cnt;
                vector<float> dis(n);
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
                        node_heap.emplace(final_min_dis, child, node, dis);
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

void MixTreeGraph::approximateKnnSearch(float *query, int k, NodeHeap &node_heap, AnsHeap &ans_heap, vector<int>& node_ids) {
    if (graph_nodes.size() < 1000) return;
    cout << "graph_nodes size " << graph_nodes.size() << endl;

    std::priority_queue<std::pair<float, size_t>> result = alg_hnsw->searchKnn(query, k / 10);  // todo k / 10 hard code
    int num_result = result.size();
    node_ids.resize(num_result);
    while (!result.empty()) {
        node_ids[-- num_result] = result.top().second;
        result.pop();
    }
    for (auto& node_id : node_ids) {
        Node* node = graph_nodes[node_id];
        if (node->type == NodeType::VNode) {
            VNode* v_node = (VNode*) node;
            v_node->has_search = true;
            float pq_dis = calc_dis(db->dimension, query, v_node->father_pivot);
            search_calc_cnt ++;
            for (int i = v_node->start; i <= v_node->end; i ++ ) {
                if (ans_heap.size() >= k && fabs(v_node->cache_dis[i - v_node->start] - pq_dis) > ans_heap.top().first) {  // todo 4 case(L2)
                    continue;
                }
                query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
                search_calc_cnt ++;
                addAns(k, query_dist[i], db->data[i], ans_heap);
            }
        }
        else {
            GNode* g_node = (GNode*) node;
            g_node->has_search = true;
            vector<float> pq_dis(g_node->cache_dis[0].size());
            for (int i = 0; i < pq_dis.size(); i ++ ) {
                pq_dis[i] = calc_dis(db->dimension, query, g_node->brother_pivots[i]);
            }
            search_calc_cnt += pq_dis.size();
            bool ok;
            for (int i = g_node->start; i <= g_node->end; i ++ ) {
                ok = true;
                for (int j = 0; j < g_node->pivot_cnt; j ++ ) {
                    if (ans_heap.size() >= k && fabs(g_node->cache_dis[i - g_node->start][j] - pq_dis[j]) > ans_heap.top().first) {  // todo 4 case(L2)
                        ok = false;
                        break;
                    }
                }

                if (!ok) continue;
                query_dist[i] = calc_dis(db->dimension, query, db->data[i]);
                search_calc_cnt ++;
                addAns(k, query_dist[i], db->data[i], ans_heap);
            }
        }
    }
}

void MixTreeGraph::knnSearchImp(float* query, int k, NodeHeap &node_heap, AnsHeap &ans_heap) {
    vector<int> node_ids;
    approximateKnnSearch(query, k, node_heap, ans_heap, node_ids);
    exactKnnSearch(query, k, node_heap, ans_heap, node_ids);
    for (auto& node_id : node_ids) {
        Node* node = graph_nodes[node_id];
        if (node->type == NodeType::VNode) {
            VNode* v_node = (VNode*) node;
            v_node->has_search = false;
        }
        else {
            GNode* g_node = (GNode*) node;
            g_node->has_search = false;

        }
    }
}

