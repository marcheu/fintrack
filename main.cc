#include "includes.h"
#include "loader.h"
#include "monte_carlo.h"
#include "portfolio.h"
#include "stochastic_optimization.h"
#include "util.h"

void print_char (float v)
{
	if (v >= 8.f / 8.f)
		printf ("█");
	else if (v >= 7.f / 8.f)
		printf ("▉");
	else if (v >= 6.f / 8.f)
		printf ("▊");
	else if (v >= 5.f / 8.f)
		printf ("▋");
	else if (v >= 4.f / 8.f)
		printf ("▌");
	else if (v >= 3.f / 8.f)
		printf ("▍");
	else if (v >= 2.f / 8.f)
		printf ("▎");
	else if (v >= 1.f / 8.f)
		printf ("▏");
	else
		printf (" ");
}

void print_histogram (std::vector < float >values)
{
	const int num_buckets = 50;
	float buckets[num_buckets];

	float min_value = FLT_MAX;
	float max_value = -FLT_MAX;
	float expectancy = 0.f;
	for (unsigned i = 0; i < values.size (); i++) {
		if (values[i] < min_value)
			min_value = values[i];
		if (values[i] > max_value)
			max_value = values[i];
		expectancy += values[i];
	}

	expectancy /= (float) values.size ();

	printf ("Value range %f - %f\n", min_value, max_value);

	min_value = 0.f;
	max_value = 3.f;

	for (int b = 0; b < num_buckets; b++)
		buckets[b] = 0.f;

	int break_even_bucket = (int) ((float) num_buckets * (1.0f - min_value) / (max_value - min_value));

	for (unsigned i = 0; i < values.size (); i++) {
		int bucket_index = (int) (num_buckets * (values[i] - min_value) / (max_value - min_value));
		// Fold the tail of the distribution.
		if (bucket_index >= num_buckets)
			bucket_index = num_buckets - 1;
		buckets[bucket_index] += 0.00012f;
	}

	float total = 0.00012f * values.size ();
	float counted = 0.f;
	for (int b = 0; b < num_buckets; b++) {
		float value = buckets[b];
		counted += value;
		if (b < break_even_bucket)
			printf (COLOR_RED);
		else if (b == break_even_bucket)
			printf (COLOR_YELLOW);
		else
			printf (COLOR_GREEN);
		int printed_char = 0;
		while (value > 0.f) {
			print_char (value);
			value -= 1.f;
			printed_char++;
		}

		while (printed_char < 120) {
			printed_char++;
			printf (" ");
		}
		printf ("| %.0f%% e=%f", 100.f * counted / total, (float) b / (float) num_buckets * (max_value - min_value) + min_value);
		printf ("\n");
	}
	printf (COLOR_NORMAL);
}

void print_correlation_matrix (std::vector < data_series > &data)
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

int main (int argc, char *argv[])
{
	srand (time (NULL));
	char *read_filename = NULL, *write_filename = NULL, *frontier_filename = NULL;

	bool need_optimize = false, need_learn = false, need_read = false, need_write = false, need_evaluate = false, need_frontier = false;

	for (int i = 1; i < argc; i++) {
		if (!strcmp (argv[i], "-o"))
			need_optimize = true;
		else if (!strcmp (argv[i], "-l"))
			need_learn = true;
		else if (!strcmp (argv[i], "-r")) {
			need_read = true;
			i++;
			read_filename = argv[i];
		}
		else if (!strcmp (argv[i], "-w")) {
			need_write = true;
			i++;
			write_filename = argv[i];
		}
		else if (!strcmp (argv[i], "-e"))
			need_evaluate = true;
		else if (!strcmp (argv[i], "-f")) {
			need_frontier = true;
			i++;
			frontier_filename = argv[i];
		}
	}

	std::vector < data_series > data;
	loader l;
	if (need_evaluate)
		l.load_all_series (data, true);
	else
		l.load_all_series (data, false);

	portfolio p;
	if (need_read) {
		p.read (read_filename, data);
		p.normalize ();
		p.print (data);
	}
	else {
		p.randomize (data);
		p.normalize ();
	}

	if (need_frontier) {
		FILE *f = fopen (frontier_filename, "a");
		portfolio single;
		int num_rounds = 16384;
		float expectancy, standard_deviation, downside_deviation;
		monte_carlo m (data, true);

		for (int i = 0; i < 100000; i++) {
			single.randomize (data);
			single.normalize ();
			m.run (single, expectancy, standard_deviation, downside_deviation, num_rounds);
			fprintf (f, "%f, %f, %f\n", expectancy, standard_deviation, downside_deviation);
			printf ("%d/100000\n", i);
		}
		fclose (f);
	}

	if (need_learn)
		stochastic_optimization (data, p, false);
	if (need_optimize)
		stochastic_optimization (data, p, true);

	if (need_evaluate) {
		float expectancy, standard_deviation, downside_deviation;
		monte_carlo m (data, true);

		int num_rounds = 32768 * 16;
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
		print_correlation_matrix (data);

		num_rounds = 32768 * 128;

		std::vector < float >values;
		for (int i = 0; i < 4; i++) {
			int y = ((int[]) { 1, 2, 4, 10 })[i];
			m.run_with_data (p, values, expectancy, standard_deviation, downside_deviation, num_rounds, y * 253);
			printf ("%02d years: e = %f σ = %f σd = %f \n", y, expectancy, standard_deviation, downside_deviation);
			print_histogram (values);
			values.clear ();
		}

	}

	if (need_write) {
		p.write (write_filename);
	}
	return 0;
}
