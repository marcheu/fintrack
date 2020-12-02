#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "data_series.h"
#include "includes.h"
#include "portfolio.h"

class monte_carlo {
public:
	monte_carlo(std::vector<data_series> & historical_data);
	void run(portfolio &p, float &expectancy, float &standard_deviation);

private:
	std::vector<data_series> & historical_data_;

	float* gpu_historical_data_;
	float* gpu_portfolio_;

	// no copy
	monte_carlo(const monte_carlo&);
};

#endif
