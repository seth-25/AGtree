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
#endif
#ifdef L2
static int func_type = 2;
#endif