#pragma once

#include "defs.h"


class DB {
public:
    ~DB() {
        for (int i = 0; i < num_data; i++) {
            delete[] data[i];
        }
        delete[] data;
        for (int i = 0; i < num_queries; i++) {
            delete[] queries[i];
        }
        delete[] queries;
        delete[] radius;
    }

    float l1_distance(const float *x, const float *y) const;

    float l2_distance(const float *x, const float *y) const;

    const char *data_filename;
    const char *query_filename;

    Method method;
    int crack_threshold;
    int tree_threshold;

    int num_data;
    int num_queries;
    int dimension;


#ifdef L1
    float **data;
    float **queries;
    float *radius;

#elif L2
    float **data;
    float **queries;
    float *radius;
#elif ED

#endif
};


namespace common {
    DB *&the_db();

    void parse_parameter(int argc, char **argv);

    void load_data();

    void load_queries();
}