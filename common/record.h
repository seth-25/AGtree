#pragma once

#include <chrono>
#define ClockType std::chrono::high_resolution_clock
#define MicroSeconds std::chrono::microseconds

extern ClockType::time_point total_begin, per_query_begin;
extern long total_time, per_query_time;
#define total_start total_begin = ClockType::now();
#define total_end total_time = std::chrono::duration_cast<MicroSeconds>(ClockType::now() - total_begin).count();
#define print_total_time cout << "Total time " << (double)total_time / MicroSeconds::period::den << endl;

#define per_query_start per_query_begin = ClockType::now();
#define per_query_end per_query_time = std::chrono::duration_cast<MicroSeconds>(ClockType::now() - per_query_begin).count();
#define print_per_query_time cout << "Per query time " << (double)per_query_time / MicroSeconds::period::den << endl;

extern ClockType::time_point crack_begin;
extern long crack_time, total_crack_time;
#define crack_start crack_begin = ClockType::now();
#define crack_end crack_time = std::chrono::duration_cast<MicroSeconds>(ClockType::now() - crack_begin).count(), total_crack_time += crack_time;
#define print_crack_time cout << "crack time " << (double)total_crack_time / MicroSeconds::period::den << endl;

extern ClockType::time_point search_begin;
extern long search_time, total_search_time;
#define search_start search_begin = ClockType::now();
#define search_end search_time = std::chrono::duration_cast<MicroSeconds>(ClockType::now() - search_begin).count(), total_search_time += search_time;
#define print_search_time cout << "knnSearch time " << (double)total_search_time / MicroSeconds::period::den << endl;


extern unsigned long search_calc_cnt;
extern unsigned long crack_calc_cnt;
extern unsigned long total_search_calc_cnt;
extern unsigned long total_crack_calc_cnt;
