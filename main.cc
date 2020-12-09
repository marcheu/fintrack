#include "includes.h"
#include "loader.h"
#include "monte_carlo.h"
#include "portfolio.h"
#include "stochastic_optimization.h"
#include "util.h"

void print_char(float v)
{
	if (v >= 8.f/8.f)
		printf("█");
	else if (v >= 7.f/8.f)
		printf("▉");
	else if (v >= 6.f/8.f)
		printf("▊");
	else if (v >= 5.f/8.f)
		printf("▋");
	else if (v >= 4.f/8.f)
		printf("▌");
	else if (v >= 3.f/8.f)
		printf("▍");
	else if (v >= 2.f/8.f)
		printf("▎");
	else if (v >= 1.f/8.f)
		printf("▏");
	else
		printf(" ");
}

void print_histogram(std::vector<float> values)
{
	const int num_buckets = 80;
	float buckets[num_buckets];

	float min_value = FLT_MAX;
	float max_value = -FLT_MAX;
	float expectancy = 0.f;
	for(unsigned i = 0; i < values.size(); i++) {
		if (values[i] < min_value)
			min_value = values[i];
		if (values[i] > max_value)
			max_value = values[i];
		expectancy += values[i];
	}

	expectancy /= (float)values.size();

	printf("value range %f - %f\n", min_value, max_value);

	for(int b = 0; b < num_buckets; b++)
		buckets[b] = 0.f;

	int expectancy_bucket = (int)((float)num_buckets * (expectancy - min_value) / (max_value - min_value));

	for(unsigned i = 0; i < values.size(); i++) {
		buckets[(int)(num_buckets * (values[i] - min_value) / (max_value - min_value))] += 0.00004f;
	}

	float total = 0.00004f * values.size();
	float counted = 0.f;
	for(int b = 0; b < num_buckets; b++) {
		float value = buckets[b];
		counted += value;
		if (b < expectancy_bucket)
			printf(COLOR_RED);
		else
			printf(COLOR_GREEN);
		while(value > 0.f) {
			print_char(value);
			value -= 1.f;
		}
		printf("\n");
		if (counted > 0.98 * total)
			break;
	}
	printf(COLOR_NORMAL);
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	char* read_filename, * write_filename;

	bool need_optimize = false, need_learn = false, need_read = false, need_write = false, need_evaluate = false;

	for(int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-o"))
			need_optimize = true;
		else if (!strcmp(argv[i], "-l"))
			need_learn = true;
		else if (!strcmp(argv[i], "-r")) {
			need_read = true;
			i++;
			read_filename = argv[i];
		}
		else if (!strcmp(argv[i], "-w")) {
			need_write = true;
			i++;
			write_filename = argv[i];
		}
		else if (!strcmp(argv[i], "-e"))
			need_evaluate = true;
	}

	std::vector<data_series> data;
	loader l;
	l.load_all_series(data);

	portfolio p;
	if (need_read) {
		p.read(read_filename, data);
		p.normalize();
		p.print(data);
	} else {
		p.randomize(data);
		p.normalize();
	}

	if (need_learn)
		stochastic_optimization(data, p, false);
	if (need_optimize)
		stochastic_optimization(data, p, true);

	if (need_evaluate) {
		float expectancy, standard_deviation;
		monte_carlo m(data, true);
		int num_rounds = 32768 * 128;
		//printf("Overall e = %f σ = %f \n", expectancy, standard_deviation);

		std::vector<float> values;
		for(int y = 1; y <= 10; y += 3) {
			m.run_with_data(p, values, expectancy, standard_deviation, num_rounds, y * 253);
			printf("%02d years: e = %f σ = %f e(84%%) = %f\n", y, expectancy, standard_deviation, expectancy - standard_deviation);
			print_histogram(values);
			values.clear();
		}

		num_rounds = 32768 * 16;
		for(unsigned i = 0; i < data.size(); i++) {
			if (p.proportions[i] > 0.0f) {
				portfolio single = p;
				for(unsigned j = 0; j < data.size(); j++)
					if (j != i)
						single.proportions[j] = 0.f;
				single.normalize();
				m.run(single, expectancy, standard_deviation, num_rounds);
				printf("  %5s e = %f σ = %f \n", data[i].name, expectancy, standard_deviation);
			}
		}
	}

	if (need_write) {
		p.write(write_filename);
	} 
	return 0;
}

