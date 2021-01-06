
#include "monte_carlo.h"

#include <curand.h>
#include <curand_kernel.h>

monte_carlo::monte_carlo (std::vector < data_series > &historical_data, bool use_gpu):historical_data_ (historical_data), use_gpu_ (use_gpu)
{
	if (use_gpu_) {
		int num_stocks = historical_data_.size ();
		cudaMallocManaged (&gpu_historical_data_, historical_data.size () * historical_data[0].size * sizeof (float));
		for (unsigned stock = 0; stock < historical_data_.size (); stock++) {
			memcpy (gpu_historical_data_ + stock * historical_data[0].size, historical_data_[stock].values, sizeof (float) * historical_data_[0].size);
		}
		cudaMallocManaged (&gpu_portfolio_, num_stocks * sizeof (float));
	}
}

__global__ void run_simulation (int seed, const int num_rounds, int num_stocks, int num_days, float *historical_data, int start_day, int days_back, float *portfolio, float *expectancy_list)
{
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	curandState_t state;

	curand_init (seed, idx, 0, &state);

	float p[GPU_SIMULATION_MAX_STOCKS];

	memcpy (p, portfolio, num_stocks * sizeof (float));

	const int duration = MONTE_CARLO_SIMULATION_DURATION;

	int round = idx;
	int position;
	for (int i = 0; i < duration; i++) {
		position = (start_day + curand (&state) % (days_back - 1)) % num_days;

		for (unsigned stock = 0; stock < num_stocks; stock++) {
			float factor = (historical_data[stock * num_days + position + 1] / historical_data[stock * num_days + position]);
			p[stock] *= factor;
		}
	}

	float expectancy = 0.;
	for (unsigned stock = 0; stock < num_stocks; stock++)
		expectancy += p[stock];

	// We simulated half a year, but expectanty is easier to read as returns per year
	expectancy_list[round] = expectancy * expectancy;
}

void monte_carlo::run_with_data (portfolio & p, std::vector < float >&expectancy_list, float &expectancy, float &standard_deviation, float &downside_deviation, int num_rounds, int days_back)
{
	int num_days = historical_data_[0].size;
	days_back = min (num_days, days_back);
	int start_day = num_days - days_back;

	if (use_gpu_) {
		int num_stocks = historical_data_.size ();
		assert (num_stocks < GPU_SIMULATION_MAX_STOCKS);

		float *gpu_expectancy_list;
		cudaMallocManaged (&gpu_expectancy_list, num_rounds * sizeof (float));

		memcpy (gpu_portfolio_, p.proportions, num_stocks * sizeof (float));
		run_simulation <<< num_rounds / 256, 256 >>> (rand (), num_rounds, num_stocks, num_days, gpu_historical_data_, start_day, days_back, gpu_portfolio_, gpu_expectancy_list);

		cudaDeviceSynchronize ();

		for (int i = 0; i < num_rounds; i++)
			expectancy_list.push_back (gpu_expectancy_list[i]);

		cudaFree (gpu_expectancy_list);
	}
	else {
		const int duration = MONTE_CARLO_SIMULATION_DURATION;
		portfolio p2;

		for (int round = 0; round < num_rounds; round++) {
			p2 = p;

			int position;
			for (int i = 0; i < duration; i++) {
				position = (start_day + rand () % (days_back - 1)) % num_days;

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
	double expectancy_d = 0.f;
	for (int i = 0; i < num_rounds; i++)
		expectancy_d += expectancy_list[i];
	expectancy_d /= (double) num_rounds;
	expectancy = expectancy_d;

	// calculate standard deviation
	double standard_deviation_d = 0.f;
	for (int i = 0; i < num_rounds; i++)
		standard_deviation_d += (expectancy_list[i] - expectancy_d) * (expectancy_list[i] - expectancy_d);
	standard_deviation_d = sqrt (standard_deviation_d / (double) num_rounds);
	standard_deviation = standard_deviation_d;

	// calculate downside deviation
	double downside_deviation_d = 0.f;
	for (int i = 0; i < num_rounds; i++)
		if (expectancy_list[i] < expectancy)
			downside_deviation_d += (expectancy_list[i] - expectancy_d) * (expectancy_list[i] - expectancy_d);
	downside_deviation_d = sqrt (downside_deviation_d / (double) num_rounds);
	downside_deviation = downside_deviation_d;
}

void monte_carlo::run (portfolio & p, float &expectancy, float &standard_deviation, float &downside_deviation, int num_rounds, int days_back)
{
	std::vector < float >dummy_list;
	run_with_data (p, dummy_list, expectancy, standard_deviation, downside_deviation, num_rounds, days_back);
}
