#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "data_series.h"
#include "includes.h"
#include "portfolio.h"


void monte_carlo(std::vector<data_series> & historical_data, portfolio &p, float &expectancy, float &standard_deviation);

#endif
