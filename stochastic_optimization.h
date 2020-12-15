#ifndef _GRADIENT_DESCENT_H_
#define _GRADIENT_DESCENT_H_

#include "data_series.h"
#include "portfolio.h"

void stochastic_optimization (std::vector < data_series > &historical_data, portfolio & p, bool refine);

#endif
