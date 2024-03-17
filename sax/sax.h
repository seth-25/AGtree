#pragma once

#include <cassert>
#include "defs.h"
#include "sax_bsearch.h"

void paa_from_ts(const float *ts_in, float *paa, int segments, int num_per_segment);

void sax_from_ts(const float *ts_in, sax_type* sax_out, int segments, int num_per_segment);

/**
 * 计算sax下界距离,不同于isax,这边的sax都是满的BIT_CARDINALITY位
 * 查的表sax_a需要固定BIT_CARDINALITY = 8
 */
float min_dist_paa_to_sax(const float *paa, const sax_type* sax_, int segments, int num_per_segment);