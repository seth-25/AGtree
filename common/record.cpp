#include "record.h"

ClockType::time_point total_begin, per_query_begin;
long total_time, per_query_time;
ClockType::time_point crack_begin;
long crack_time, total_crack_time;
ClockType::time_point search_begin;
long search_time, total_search_time;

unsigned long search_calc_cnt = 0;
unsigned long crack_calc_cnt = 0;
unsigned long total_search_calc_cnt = 0;
unsigned long total_crack_calc_cnt = 0;