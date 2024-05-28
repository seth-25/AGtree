
#include "hnswalg.h"

#include <iostream>
#include <queue>
#include <cstring>
int main() {
    int N = 500, D = 100, K = 100;

    //////////////////////
    int M = std::min(64, D);
    int efConstruction = 256;
    //////////////////////
    hnswlib::L2Space l2space(D);
    hnswlib::HierarchicalNSW<float>* alg_hnsw = new hnswlib::HierarchicalNSW<float>(&l2space, N, M, efConstruction);
    for (int i = 0; i < N; i++) {
        alg_hnsw->addPoint(data[i], i);
    }

    std::cout << "ok" << std::endl;

    scanf("%d", &K);
    ////////////////////
    int k_max = std::min(20 * K, N);
    ////////////////////
    char buffer[50];
    float query[D];
    while(true) {
        if (scanf("%s", buffer) == EOF)
            break;
        if (!strcmp(buffer, "end"))
            break;
        sscanf(buffer, "%f", &query[0]);
        for (int i = 1; i < D; i ++ ) {
            scanf("%f", &query[i]);
        }

        std::priority_queue<std::pair<float, size_t>> result = alg_hnsw->searchKnn(query, k_max);
        for (int i = 0; i < k_max - K; i ++ ) {
            result.pop();
        }
        while (!result.empty()) {
            printf("%lu ",result.top().second);
            result.pop();
        }

        std::cout << std::endl;
    }


    for (int i = 0; i < N; i ++ ) {
        delete[] data[i];
    }
    delete[] data;
}