#include "db.h"
#include <vector>
#include <algorithm>
#include "sax.h"
#include <unordered_map>
#include <queue>
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

    priority_queue<float> ans_dis;

    std::unordered_map<float*, sax_type*> sax_map;
    for (int i = 0; i < db->num_data; i ++ ) {
        sax_type* sax = new sax_type[db->segment];
        sax_from_ts(db->data[i], sax, db->segment, db->num_per_segment);
        sax_map[db->data[i]] = sax;
    }
    int k = 100;
    long cnt = 0;
    for (int i = 0; i < db->num_queries; i ++ ) {
//    for (int i = 0; i < 1; i ++ ) {
        float query_paa[db->segment];
        paa_from_ts(db->queries[i], query_paa, db->segment, db->num_per_segment);
        for (int j = 0; j < db->num_data; j ++ ) {
            float sax_dis = min_dist_paa_to_sax(query_paa, sax_map[db->data[j]], db->segment, db->num_per_segment);
            if (ans_dis.size() == k && sax_dis > ans_dis.top()) continue;
            float dis = calc_dis(db->dimension, db->data[j], db->queries[i]);
            cnt ++;
            if (ans_dis.size() < k) {
                ans_dis.emplace(dis);
            }
            else {
                if (dis < ans_dis.top()) {
                    ans_dis.pop();
                    ans_dis.emplace(dis);
                }
            }
        }
        cout << i << ": " << ans_dis.size() << endl;
        while(!ans_dis.empty()) {
            cout << ans_dis.top() << " " ;
            ans_dis.pop();
        }
        cout << endl;
        long total = db->num_queries * db->num_data;
        cout << "calc cnt:" << cnt << " total:" << total << " per:" << (float)cnt/(float)total << endl;
        cnt = 0;
    }
//    0.820133
}