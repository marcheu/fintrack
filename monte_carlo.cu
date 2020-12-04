
#include "monte_carlo.h"

#include <curand.h>
#include <curand_kernel.h>

monte_carlo::monte_carlo(std::vector<data_series> & historical_data, bool use_gpu)
	: historical_data_ (historical_data), use_gpu_ (use_gpu)
{
	if (use_gpu_) {
		int num_stocks = historical_data_.size();
		cudaMallocManaged(&gpu_historical_data_, historical_data.size() * historical_data[0].size * sizeof(float));
		for(unsigned stock = 0; stock < historical_data_.size(); stock++) {
			memcpy(gpu_historical_data_ + stock * historical_data[0].size, historical_data_[stock].values, sizeof(float) * historical_data_[0].size);
		}
		cudaMallocManaged(&gpu_portfolio_, num_stocks * sizeof(float));
	}
}

__global__ void run_simulation(int seed, const int num_rounds, int num_stocks, int num_days, float *historical_data, float *portfolio, float *expectancy_list)
{
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	curandState_t state;

	/* we have to initialize the state */
	curand_init(seed, /* the seed controls the sequence of random values that are produced */
			idx, /* the sequence number is only important with multiple cores */
			0, /* the offset is how much extra we advance in the sequence for each call, can be 0 */
			&state);

	float p[512];

	memcpy(p, portfolio, num_stocks * sizeof(float));

	const int duration = 253 * 1; // 1 years @ 253 trading days per year

	int round = idx;
	for(int i = 0; i < duration; i++) {
		int position = curand(&state) % (num_days - 1);

		for(unsigned stock = 0; stock < num_stocks; stock++) {
			float factor = (historical_data[stock * num_days + position + 1] / historical_data[stock * num_days + position]);
			p[stock] *= factor;
		}
	}

	float expectancy = 0.;
	for(unsigned stock = 0; stock < num_stocks; stock++)
		expectancy += p[stock];

	expectancy_list[round] = expectancy;
}

void monte_carlo::run(portfolio &p, float &expectancy, float &standard_deviation, int num_rounds)
{
	if (use_gpu_) {
		int num_stocks = historical_data_.size();
		int num_days = historical_data_[0].size;

		float *expectancy_list;
		cudaMallocManaged(&expectancy_list, num_rounds * sizeof(float)); 

		memcpy(gpu_portfolio_, p.proportions, num_stocks * sizeof(float));
		run_simulation<<<num_rounds / 256, 256>>>(rand(), num_rounds, num_stocks, num_days, gpu_historical_data_, gpu_portfolio_, expectancy_list);

		cudaDeviceSynchronize();

		// calculate expectancy
		expectancy = 0.f;
		for(int i = 0; i < num_rounds; i++) {
			expectancy += expectancy_list[i];
		}
		expectancy /= (float) num_rounds;

		// calculate standard deviation
		standard_deviation = 0.f;
		for(int i = 0; i < num_rounds; i++)
			standard_deviation += (expectancy_list[i] - expectancy) * (expectancy_list[i] - expectancy);
		standard_deviation = sqrtf(standard_deviation / (float) num_rounds);
	} else {
		const int duration = 253 * 1; // 1 years @ 253 trading days per year
		std::vector<float> expectancy_list;
		portfolio p2;

		for(int round = 0; round < num_rounds; round++) {
	 		p2 = p;

			for(int i = 0; i < duration; i++) {
				int position = rand() % (historical_data_[0].size - 1);
	
				for(unsigned stock = 0; stock < historical_data_.size(); stock++) {
					float factor = (historical_data_[stock].values[position + 1] / historical_data_[stock].values[position]);
					p2.proportions[stock] *= factor;
				}
			}

			float round_expectancy = 0.;
			for(unsigned stock = 0; stock < historical_data_.size(); stock++)
				round_expectancy += p2.proportions[stock];
			expectancy_list.push_back(round_expectancy);
		}

		// calculate expectancy
		expectancy = 0.f;
		for(unsigned i = 0; i < expectancy_list.size(); i++)
			expectancy += expectancy_list[i];
		expectancy /= (float) expectancy_list.size();

		// calculate standard deviation
		standard_deviation = 0.f;
		for(unsigned i = 0; i < expectancy_list.size(); i++)
			standard_deviation += (expectancy_list[i] - expectancy) * (expectancy_list[i] - expectancy);
		standard_deviation = sqrtf(standard_deviation / (float) expectancy_list.size());
	}
}

