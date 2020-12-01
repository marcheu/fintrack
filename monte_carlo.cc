
#include "monte_carlo.h"

void monte_carlo(std::vector<data_series> &historical_data, portfolio &p, float &expectancy, float &standard_deviation)
{
	const int duration = 253 * 1; // 1 years @ 253 trading days per year
	std::vector<float> expectancy_list;
	std::vector<float> standard_deviation_list;
	portfolio p2;

	for(int round = 0; round < 20000; round++) {
 		p2 = p;

		for(int i = 0; i < duration; i++) {
			int position = rand() % (historical_data[0].size - 1);

			for(unsigned stock = 0; stock < historical_data.size(); stock++) {
				float factor = (historical_data[stock].values[position + 1] / historical_data[stock].values[position]);
				p2.proportions[stock] *= factor;
			}
		}

		float round_expectancy = 0.;
		for(unsigned stock = 0; stock < historical_data.size(); stock++)
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

