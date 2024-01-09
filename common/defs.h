#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>

enum class Method {
    STANDARD,

};

#ifdef ED
static int func_type = 0;
#endif
#ifdef L1
static int func_type = 1;
#define dist(db, data, query) (db->l1_distance(data, query))
#endif
#ifdef L2
static int func_type = 2;
#define dist(db, data, query) (db->l2_distance(data, query))
#endif