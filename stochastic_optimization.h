#ifndef _STOCHASTIC_OPTIMIZATION_H_
#define _STOCHASTIC_OPTIMIZATION_H_

#include "data_series.h"
#include "portfolio.h"

void stochastic_optimization (std::vector < data_series > &historical_data, portfolio & p, bool refine, int days_back, float goal);

#endif
