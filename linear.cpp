#include "db.h"
#include <vector>
#include <algorithm>
#include "record.h"
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

    vector<float> ans_dis;

    int total_ans = 0;
    total_start
    for (int i = 0; i < db->num_queries; i ++ ) {
        for (int j = 0; j < db->num_data; j ++ ) {
            float dis = calc_dis(db->dimension, db->data[j], db->queries[i]);
            total_search_calc_cnt ++;
            if (dis <= db->radius[i]) {
                ans_dis.emplace_back(dis);
            }
        }
        cout << i << ": " << ans_dis.size() << endl;
        total_ans += ans_dis.size();
//        sort(ans_dis.begin(), ans_dis.end());
//        for (int j = 0; j < ans_dis.size(); j ++ ) {
//            cout << ans_dis[j] << " " ;
////            cout << j << " " << ans_dis[j] << endl;
//        }
//        cout << endl << endl;
        ans_dis.clear();
    }
    total_end
    print_total_time
    cout << total_ans << " " << total_search_calc_cnt << endl;

}