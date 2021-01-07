#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "constants.h"
#include "data_series.h"
#include "includes.h"
#include "portfolio.h"

class monte_carlo {
      public:
	monte_carlo (std::vector < data_series > &historical_data, bool use_gpu);
	~monte_carlo ();
	void run_with_data (portfolio & p, std::vector < float >&expectancy_list, float &expectancy, float &standard_deviation, float &downside_deviation, int num_rounds, int days_back = TRADING_DAYS_PER_YEAR * 4);
	void run (portfolio & p, float &expectancy, float &standard_deviation, float &downside_deviation, int num_rounds, int days_back = TRADING_DAYS_PER_YEAR * 4);

      private:
	  std::vector < data_series > &historical_data_;

	bool use_gpu_;

	float *gpu_historical_data_;
	float *gpu_portfolio_;

	// no copy
	  monte_carlo (const monte_carlo &);
};

#endif
