#include <algorithm>
#include "db.h"
#include "mix_tree.h"
#include "record.h"
#include "mix_tree_cache.h"
#include "mix_tree_kmeans.h"

using namespace std;

int main(int argc, char **argv) {
    common::parse_parameter(argc, argv);
    DB *db = common::the_db();

    cout << "Point dataset : " << db->data_filename << endl;
    cout << "Query dataset : " << db->query_filename << endl;
    cout << "Threshold : "<< db->tree_threshold << " " << db->crack_threshold << endl;

    common::load_data();
    common::load_queries();
    cout << "Number data : " << db->num_data << endl;
    cout << "Number queries : " << db->num_queries << endl;

    AnsHeap ans_dis;
    int total_ans = 0;
    MixTree* mixtree;
    switch (db->method) {
        case Method::CACHE:
            mixtree = new MixTreeCache(db);
            break;
        case Method::KMEANS:
            mixtree = new MixTreeKmeans(db);
            break;
        default:
            cout << "method not found" << endl;
            return 0;
    }

//    float dis = calc_dis(db->dimension, db->queries[0], db->queries[1]);
//    cout << "dis:" << dis;
//    exit(0);
    total_start
    for (int i = 0; i < db->num_queries; i ++ ) {
        per_query_start
        mixtree->knnSearch(db->queries[i % db->num_queries], db->K, ans_dis);
        per_query_end
        cout << i + 1 << "\t" << ans_dis.size() << "\t";

        cout << "knnSearch: " << search_calc_cnt << ", crack: " << crack_calc_cnt << ", total: " << search_calc_cnt + crack_calc_cnt << endl;
        total_search_calc_cnt += search_calc_cnt; total_crack_calc_cnt += crack_calc_cnt;
        search_calc_cnt = 0, crack_calc_cnt = 0;
        total_ans += ans_dis.size();
        print_per_query_time
        while(!ans_dis.empty()) {
            cout << ans_dis.top().first << " ";
            ans_dis.pop();
        }
        cout << endl << endl;
        ans_dis = AnsHeap();
    }
    total_end

    print_search_time
    print_crack_time
    print_total_time
    cout << "Total ans " << total_ans << endl;
    cout << "knnSearch: " << total_search_calc_cnt << ", crack: " << total_crack_calc_cnt << ", total: " << total_search_calc_cnt + total_crack_calc_cnt << endl;
}