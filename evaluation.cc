#include "constants.h"
#include "evaluation.h"
#include "monte_carlo.h"
#include "util.h"

static void print_correlation_matrix (std::vector < data_series > &data)
{
	std::vector < float >mean_values;

	for (unsigned i = 0; i < data.size (); i++) {
		float mean;

		mean = 0.f;
		for (int j = 0; j < data[i].size; j++)
			mean += data[i].values[j];
		mean /= (float) data[i].size;

		mean_values.push_back (mean);
	}

	printf ("=============================================\n      ");
	for (unsigned s1 = 0; s1 < data.size (); s1++)
		printf ("%5s  ", data[s1].name);
	printf ("\n");
	for (unsigned s1 = 0; s1 < data.size (); s1++) {
		printf ("%5s  ", data[s1].name);
		for (unsigned s2 = 0; s2 < data.size (); s2++) {
			float cov;

			cov = 0.f;
			for (int i = 0; i < data[0].size; i++)
				cov += (data[s1].values[i] - mean_values[s1]) * (data[s2].values[i] - mean_values[s2]);

			float coef1 = 0.0f, coef2 = 0.0f;
			for (int i = 0; i < data[0].size; i++)
				coef1 += (data[s1].values[i] - mean_values[s1]) * (data[s1].values[i] - mean_values[s1]);
			coef1 = sqrtf (coef1);

			for (int i = 0; i < data[0].size; i++)
				coef2 += (data[s2].values[i] - mean_values[s2]) * (data[s2].values[i] - mean_values[s2]);
			coef2 = sqrtf (coef2);

			cov /= (float) (coef1 * coef2);

			if (cov > 0.3)
				printf (COLOR_GREEN);
			else if (cov < -0.3)
				printf (COLOR_RED);
			else
				printf (COLOR_NORMAL);
			printf ("% .3f ", cov);
		}
		printf (COLOR_NORMAL);
		printf ("\n");
	}
	printf ("=============================================\n");
}

void evaluate_portfolio (std::vector < data_series > &data, portfolio & p)
{
	float expectancy, standard_deviation, downside_deviation;
	monte_carlo m (data, true);

	// evaluate each component
	int num_rounds = 1 << 19;
	for (unsigned i = 0; i < data.size (); i++) {
		if (p.proportions[i] > 0.0f) {
			portfolio single = p;
			for (unsigned j = 0; j < data.size (); j++)
				if (j != i)
					single.proportions[j] = 0.f;
			single.normalize ();
			m.run (single, expectancy, standard_deviation, downside_deviation, num_rounds);
			printf ("  %5s e = %f σ = %f σd = %f \n", data[i].name, expectancy, standard_deviation, downside_deviation);
		}
	}

	// print correlation matrix
	print_correlation_matrix (data);

	num_rounds = 1 << 22;

	// evaluate at 0.5 1 2 4 10 years
	std::vector < float >values;
    int intervals[] = { TRADING_DAYS_PER_YEAR / 2, 1 * TRADING_DAYS_PER_YEAR, 2 * TRADING_DAYS_PER_YEAR, 4 * TRADING_DAYS_PER_YEAR, 10 * TRADING_DAYS_PER_YEAR };
	for (int i = 0; i < 5; i++) {
		int y = intervals[i];
		m.run_with_data (p, values, expectancy, standard_deviation, downside_deviation, num_rounds, y);
		printf ("%02f years: e = %f σ = %f σd = %f \n", y / (float) TRADING_DAYS_PER_YEAR, expectancy, standard_deviation, downside_deviation);
		print_histogram (values);
		values.clear ();
	}

	// evaluate sectors
	float sectors[NUM_SECTORS];
	for (unsigned s = 0; s < NUM_SECTORS; s++)
		sectors[s] = 0.f;

	for (unsigned t = 0; t < data.size (); t++) {
		for (unsigned s = 0; s < NUM_SECTORS; s++) {
			sectors[s] += p.proportions[t] * data[t].leverage * data[t].sector_proportions[s];
		}
	}

	float sum = 0.f;
	for (unsigned s = 0; s < NUM_SECTORS; s++) {
		sum += sectors[s];
		printf ("%s: %6.2f ", sector_name[s], sectors[s]);
		printf (COLOR_BLUE);
		print_bar (sectors[s]);
		printf (COLOR_NORMAL);
		printf ("\n");
	}
	printf ("Total %f\n", sum);
}
