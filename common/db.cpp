#include "db.h"

#include <fstream>
#include <cmath>
#include "getopt.h"

namespace common {

    void usage()
    {
        std::cout << "Useage " << std::endl;
        std::cout << "-s: " << std::endl;
    }

    DB *&the_db() {
        static auto *db = new DB();
        return db;
    }

    void parse_parameter(int argc, char **argv) {
        DB *db = the_db();
        int opt;
        while ((opt = getopt(argc, argv, "t:T:csk")) > 0) {
            switch (opt) {
                case 't':
                    db->crack_threshold = atoi(optarg);
                    break;
                case 'T':
                    db->tree_threshold = atoi(optarg);
                    break;
                case 's':
                    db->method = Method::SAX;
                    break;
                case 'c':
                    db->method = Method::CACHE;
                    break;
                case 'k':
                    db->method = Method::KMEANS;
                    break;
                case 'h':
                    usage();
                    exit(0);
                default:
                    std::cout << "Unknown option: " << static_cast<char>(opt) << ", ignored" << std::endl;
                    break;
            }
        }
        db->data_filename = argv[optind];
        db->query_filename = argv[optind + 1];
    }

    void load_data() {
        DB *db = the_db();
        std::ifstream file(db->data_filename);
        if (!file.is_open()) {
            std::cerr << "Can not open " << db->data_filename << std::endl;
            return;
        }
        int func;
        file >> db->dimension >> db->num_data >> func >> db->segment;
        if (db->dimension % db->segment != 0) {
            std::cerr << "Segment not correct." << std::endl;
            return;
        }
        db->num_per_segment = db->dimension / db->segment;
        if (func != func_type) {
            std::cerr << "Distance function type not match." << std::endl;
            return;
        }

        db->data = new float * [db->num_data];
        for (int i = 0; i < db->num_data; i ++ ) {
            db->data[i] = new float [db->dimension];
            for (int j = 0; j < db->dimension; j ++ ) {
                if (!(file >> db->data[i][j])) {
                    std::cerr << "Data not enough." << std::endl;
                    break;
                }
            }
        }
        file.close();
    }

    void load_queries() {
        DB *db = the_db();
        std::ifstream file(db->query_filename);
        if (!file.is_open()) {
            std::cerr << "Can not open " << db->query_filename << std::endl;
            return;
        }
        int num;
        file >> num;
        db->num_queries = num;
        db->queries = new float * [num];
        db->radius = new float [num];
        for (int i = 0; i < num; i ++ ) {
            db->queries[i] = new float [db->dimension];
            file >> db->radius[i];
            for (int j = 0; j < db->dimension; j ++ ) {
                if (!(file >> db->queries[i][j])) {
                    std::cerr << "Queries not enough." << std::endl;
                    break;
                }
            }
        }
        file.close();
    }
}

