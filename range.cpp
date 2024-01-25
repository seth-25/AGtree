#include <algorithm>
#include "db.h"
#include "AGtree.h"
#include "time_record.h"

using namespace std;

//int main(int argc, char **argv) {
//    common::parse_parameter(argc, argv);
//    DB *db = common::the_db();
//    cout << "Point dataset : " << db->data_filename << endl;
//    cout << "Query dataset : " << db->query_filename << endl;
//    cout << "Threshold : " << db->crack_threshold << endl;
//
//    common::load_data();
//    common::load_queries();
//    cout << "Number data : " << db->num_data << endl;
//    cout << "Number queries : " << db->num_queries << endl;
//
//    float *distance = (float *)calloc(db->num_data, sizeof(float));
//    vector<float> ans_dis;
//    int total_ans = 0;
//    AGtree* agtree = new AGtree(db);
//
//
//    total_start
//    for (int i = 0; i < db->num_queries; i ++ ) {
//        per_query_start
//        agtree->searchCache(agtree->root, db->queries[i], db->radius[i], i, distance, ans_dis);
//        per_query_end
//        cout << i + 1 << "\t" << ans_dis.size() << "\t";
//        total_ans += ans_dis.size();
//        print_per_query_time
//        ans_dis.clear();
//    }
//    total_end
//    print_search_time
//    print_crack_time
//    print_total_time
//    cout << "Total ans " << total_ans << endl;
//    cout << "search: "<< search_dis_cnt << ", crack: " << crack_dis_cnt << ", total: " << search_dis_cnt + crack_dis_cnt << endl;
//}

int main(int argc, char **argv) {
    common::parse_parameter(argc, argv);
    DB *db = common::the_db();
    cout << "Point dataset : " << db->data_filename << endl;
    cout << "Query dataset : " << db->query_filename << endl;
    cout << "Threshold : " << db->crack_threshold << endl;

    common::load_data();
    common::load_queries();
    cout << "Number data : " << db->num_data << endl;
    cout << "Number queries : " << db->num_queries << endl;

    float *distance = (float *)calloc(db->num_data, sizeof(float));
    vector<float> ans_dis;
    int total_ans = 0;
    AGtree* agtree = new AGtree(db);


    total_start
    for (int i = 0; i < db->num_queries; i ++ ) {
//    for (int i = 0; i < 1; i ++ ) {
        per_query_start
//        agtree->search(agtree->root, db->queries[i], db->radius[i], i, distance, ans_dis);
//        agtree->searchMany(agtree->root, db->queries[i], db->radius[i], i, distance, ans_dis);
        agtree->searchCache(agtree->root, db->queries[i], db->radius[i], i, 0, distance, ans_dis);
        per_query_end
//        for (int j = 0; j < ans_dis.size(); j ++ ) {
//            cout << ans_dis[j] << " " ;
//        }
        cout << i + 1 << "\t" << ans_dis.size() << "\t";
        cout << "search: "<< search_dis_cnt << ", crack: " << crack_dis_cnt << ", total: " << search_dis_cnt + crack_dis_cnt << endl;
        total_search_dis_cnt += search_dis_cnt; total_crack_dis_cnt += crack_dis_cnt;
        search_dis_cnt = 0, crack_dis_cnt = 0;
        total_ans += ans_dis.size();
        print_per_query_time
        ans_dis.clear();
    }
    total_end
    print_search_time
    print_crack_time
    print_total_time
    cout << "Total ans " << total_ans << endl;
    cout << "search: "<< total_search_dis_cnt << ", crack: " << total_crack_dis_cnt << ", total: " << total_search_dis_cnt + total_crack_dis_cnt << endl;
}