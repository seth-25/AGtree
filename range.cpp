#include <algorithm>
#include "db.h"
#include "AGtree.h"
#include "time_record.h"

using namespace std;
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
    AGtree* agtree = new AGtree(db);



//    for (int i = 0; i < db->num_queries; i ++ ) {
//        agtree->search(&agtree->root, db->queries[i], db->radius[i], i, distance, ans_dis);
//        cout << "num ans " << ans_dis.size() << endl;
//        sort(ans_dis.begin(), ans_dis.end());
//        for (int j = 0; j < ans_dis.size(); j ++ ) {
//            cout << ans_dis[j] << " " ;
//        }
//        cout << endl;
//        ans_dis.clear();
//    }

//    agtree->search(agtree->root, db->queries[0], db->radius[0], 0, distance, ans_dis);
//    cout << ans_dis.size() << " ";

    total_start
    for (int i = 0; i < db->num_queries; i ++ ) {
        per_query_start
        agtree->search(agtree->root, db->queries[i], db->radius[i], i, distance, ans_dis);
        per_query_end
//        for (int j = 0; j < ans_dis.size(); j ++ ) {
//            cout << ans_dis[j] << " " ;
//        }
        cout << ans_dis.size() << " ";
        print_per_query_time
        ans_dis.clear();
    }
    total_end
    print_search_time
    print_crack_time
    print_total_time
    cout << cnt_calc_dis << endl;
}