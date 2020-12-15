#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "data_series.h"
#include "includes.h"
#include "portfolio.h"

class monte_carlo {
      public:
	monte_carlo (std::vector < data_series > &historical_data, bool use_gpu);
	void run_with_data (portfolio & p, std::vector < float >&expectancy_list, float &expectancy, float &standard_deviation, int num_rounds, int days_back = 253 * 4 /*INT_MAX */ );
	void run (portfolio & p, float &expectancy, float &standard_deviation, int num_rounds, int days_back = 253 * 4 /*INT_MAX */ );

      private:
	  std::vector < data_series > &historical_data_;

	bool use_gpu_;

	float *gpu_historical_data_;
	float *gpu_portfolio_;

	// no copy
	  monte_carlo (const monte_carlo &);
};

#endif
