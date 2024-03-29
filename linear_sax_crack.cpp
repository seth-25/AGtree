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

    vector<float> distance(db->num_data);
    int cnt;
//    for (int i = 0; i < db->num_queries; i ++ ) {
    for (int i = 0; i < 2; i ++ ) {
        for (int j = 0; j < db->num_data; j ++ ) {

            float dis = calc_dis(db->dimension, db->data[j], db->queries[i]);
            distance[j] = dis;
        }
        sort(distance.begin(), distance.end());
        cnt = 0;
        for (int j = 0; j < db->num_data; j ++ ) {
            cout << distance[j] << " ";
            if (distance[j] < 3)    cnt ++;
        }
        cout << endl << cnt << endl << endl;
    }

//    int k = 100;
//    long cnt = 0;
//    for (int i = 0; i < db->num_queries; i ++ ) {
////    for (int i = 0; i < 1; i ++ ) {
//        float * query = db->queries[i];
//        int rnd_dis[3];
//        for (int j = 0; j < 3; j ++ ) {
//            rnd_dis[j] = (rand() % db->num_data );
//            distance[rnd_dis[j]] = calc_dis(db->dimension, query, db->data[rnd_dis[j]]);
//        }
//        float med_dis = max(min(distance[rnd_dis[0]], distance[rnd_dis[1]]),
//                            min(max(distance[rnd_dis[0]], distance[rnd_dis[1]]),
//                                distance[rnd_dis[2]])
//        );
//        cout << "med dis:" << med_dis << endl;
//
//
//        float query_paa[db->segment];
//        paa_from_ts(query, query_paa, db->segment, db->num_per_segment);
//        for (int j = 0; j < db->num_data; j ++ ) {
//            float sax_dis = min_dist_paa_to_sax(query_paa, sax_map[db->data[j]], db->segment, db->num_per_segment);
////            if (sax_dis > med_dis)  continue;
//            float dis = calc_dis(db->dimension, db->data[j], db->queries[i]);
//            cout << sax_dis << " " << dis << endl;
//            cnt ++;
//        }
//        cout << i << ": " << ans_dis.size() << endl;
//        long total = db->num_data;
//        cout << "calc cnt:" << cnt << " total:" << total << " per:" << (float)cnt/(float)total << endl;
//        cnt = 0;
//    }
}