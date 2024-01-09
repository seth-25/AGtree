#pragma once

#include <ctime>

clock_t begin_total, end_total, begin_per_query, end_per_query;
#define total_start begin_total = clock();
#define total_end end_total = clock() - begin_total;
#define print_total_time cout << "Total time " << (double)end_total / CLOCKS_PER_SEC << endl;
#define per_query_start begin_per_query = clock();
#define per_query_end end_per_query = clock() - begin_per_query;
#define print_per_query_time cout << "Per query time " << (double)end_per_query / CLOCKS_PER_SEC << endl;

clock_t begin_crack, end_crack, total_crack;
#define crack_start begin_crack = clock();
#define crack_end end_crack = clock() - begin_crack, total_crack += end_crack;
#define print_crack_time cout << "crack time " << (double)total_crack / CLOCKS_PER_SEC << endl;

clock_t begin_search, end_search, total_search;
#define search_start begin_search = clock();
#define search_end end_search = clock() - begin_search, total_search += end_search;
#define print_search_time cout << "search time " << (double)total_search / CLOCKS_PER_SEC << endl;

unsigned long cnt_calc_dis = 0;
