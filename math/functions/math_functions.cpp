#include "../math.h"
#include "../../util.h"
#include "../../id/id_api.h"

/*
  These are functions that operate on math_number_set_t's, and
  NOT continuous distributions. All continuous distributions
  are handled inside of the 'dist' namespace (math::stats::dist)
*/

#pragma message("math::functions::sum_inclusive doesn't assume X is ordered, should order at least one dimension, and it should probably be the X")
