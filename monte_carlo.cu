
#include "monte_carlo.h"

#include <curand.h>
#include <curand_kernel.h>

monte_carlo::monte_carlo (std::vector < data_series > &historical_data, bool use_gpu):historical_data_ (historical_data), use_gpu_ (use_gpu)
{
#ifdef __NVCC__
	if (use_gpu_) {
		unsigned num_stocks = historical_data_.size ();
		int num_days = historical_data_[0].size;
		cudaMallocManaged (&gpu_historical_data_, historical_data.size () * historical_data[0].size * sizeof (float));
		// We pre-divide pairs of days
		for (unsigned stock = 0; stock < num_stocks; stock++)
			for (int day = 0; day < num_days - 1; day++)
				gpu_historical_data_[day * num_stocks + stock] = historical_data_[stock].values[day + 1] / historical_data_[stock].values[day];

		cudaMallocManaged (&gpu_portfolio_, num_stocks * sizeof (float));
	}
#endif
}

monte_carlo::~monte_carlo ()
{
#ifdef __NVCC__
	if (use_gpu_) {
		cudaFree (gpu_historical_data_);
		cudaFree (gpu_portfolio_);
	}
#endif
}

#ifdef __NVCC__
__global__ void run_simulation (int seed, const int num_rounds, int num_stocks, int num_days, float *historical_data, int start_day, int days_back, float *portfolio, float *expectancy_list)
{
	int idx = threadIdx.x + blockIdx.x * blockDim.x;

	if (idx >= num_rounds)
		return;

	curandState_t state;

	curand_init (seed, idx, 0, &state);

	float p[GPU_SIMULATION_MAX_STOCKS];

	memcpy (p, portfolio, num_stocks * sizeof (float));

	const int duration = MONTE_CARLO_SIMULATION_DURATION;

	int round = idx;
	int position;
	for (int i = 0; i < duration; i++) {
		position = (start_day + curand (&state) % (days_back - 1)) % (num_days - 1);

		for (unsigned stock = 0; stock < num_stocks; stock++) {
			if (p[stock] == 0.f)
				continue;
			float factor = historical_data[position * num_stocks + stock];
			p[stock] *= factor;
		}
	}

	float expectancy = 0.;
	for (unsigned stock = 0; stock < num_stocks; stock++)
		expectancy += p[stock];

	// We simulated half a year, but expectanty is easier to read as returns per year
	expectancy_list[round] = expectancy * expectancy;
}
#endif

void monte_carlo::run_with_data (portfolio & p, std::vector < float >&expectancy_list, float &expectancy, float &standard_deviation, float &downside_deviation, float &downsize_75_deviation, int num_rounds, int days_back)
{
	int num_days = historical_data_[0].size;
	days_back = min (num_days, days_back);
	int start_day = num_days - days_back;

#ifdef __NVCC__
	if (use_gpu_) {
		int num_stocks = historical_data_.size ();
		assert (num_stocks < GPU_SIMULATION_MAX_STOCKS);

		float *gpu_expectancy_list;

		// Small GPU optimization: we do rounds of 1 << 17 at most, since this is faster.
		cudaMallocManaged (&gpu_expectancy_list, (1 << 17) * sizeof (float));

		int remaining_rounds = num_rounds;
		while (remaining_rounds > 0) {
			int gpu_rounds = min (remaining_rounds, 1 << 17);

			memcpy (gpu_portfolio_, p.proportions, num_stocks * sizeof (float));
			run_simulation <<< (gpu_rounds + 255) / 256, 256 >>> (rand (), gpu_rounds, num_stocks, num_days, gpu_historical_data_, start_day, days_back, gpu_portfolio_, gpu_expectancy_list);
			cudaDeviceSynchronize ();

			for (int i = 0; i < gpu_rounds; i++)
				expectancy_list.push_back (gpu_expectancy_list[i]);
			remaining_rounds -= gpu_rounds;
		}

		cudaFree (gpu_expectancy_list);
	}
	else
#endif
	{
		const int duration = MONTE_CARLO_SIMULATION_DURATION;
		portfolio p2;

		for (int round = 0; round < num_rounds; round++) {
			p2 = p;

			int position;
			for (int i = 0; i < duration; i++) {
				position = (start_day + rand () % (days_back - 1)) % (num_days - 1);

				for (unsigned stock = 0; stock < historical_data_.size (); stock++) {
					float factor = (historical_data_[stock].values[position + 1] / historical_data_[stock].values[position]);
					p2.proportions[stock] *= factor;
				}
			}

			float round_expectancy = 0.;
			for (unsigned stock = 0; stock < historical_data_.size (); stock++)
				round_expectancy += p2.proportions[stock];

			// We simulated half a year, but expectanty is easier to read as returns per year
			expectancy_list.push_back (round_expectancy * round_expectancy);
		}
	}

	// calculate expectancy
	double expectancy_d = 0.;
	for (int i = 0; i < num_rounds; i++)
		expectancy_d += expectancy_list[i];
	expectancy_d /= (double) num_rounds;
	expectancy = expectancy_d;

	// calculate standard deviation
	double standard_deviation_d = 0.;
	for (int i = 0; i < num_rounds; i++)
		standard_deviation_d += (expectancy_list[i] - expectancy_d) * (expectancy_list[i] - expectancy_d);
	standard_deviation_d = sqrt (standard_deviation_d / (double) num_rounds);
	standard_deviation = standard_deviation_d;

	// calculate downside deviation
	double downside_deviation_d = 0.;
	for (int i = 0; i < num_rounds; i++)
		if (expectancy_list[i] < expectancy)
			downside_deviation_d += (expectancy_list[i] - expectancy_d) * (expectancy_list[i] - expectancy_d);
	downside_deviation_d = sqrt (downside_deviation_d / (double) num_rounds);
	downside_deviation = downside_deviation_d;

	// calculate downsize_75 deviation (less than 75% left, more than 25% loss)
	double downsize_75_deviation_d = 0.;
	for (int i = 0; i < num_rounds; i++)
		if (expectancy_list[i] < 0.75)
			downsize_75_deviation_d += (expectancy_list[i] - 0.75) * (expectancy_list[i] - 0.75);
	downsize_75_deviation_d = sqrt (downsize_75_deviation_d / (double) num_rounds);
	downsize_75_deviation = downsize_75_deviation_d;
}

void monte_carlo::run (portfolio & p, float &expectancy, float &standard_deviation, float &downside_deviation, float &downsize_75_deviation, int num_rounds, int days_back)
{
	std::vector < float >dummy_list;
	run_with_data (p, dummy_list, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
}
