#include "stochastic_optimization.h"
#include "monte_carlo.h"
#include "util.h"

static void randomize(portfolio &p, int size)
{
	for(int i = 0; i < size; i++) {
		p.proportions[i] = (float)(rand()%100);
	}
}

static void print_portfolio(portfolio &p, std::vector<data_series> & historical_data, int size)
{
	printf("|");
	for(int i = 0; i < size; i++)
		if (p.proportions[i] >= 0.003)
			printf("%-5s|", historical_data[i].name);
	printf("\n|");
	for(int i = 0; i < size; i++) {
		if (p.proportions[i] < 0.003)
			continue;

		if (p.proportions[i] > 0.4)
		printf(COLOR_RED);
		else if (p.proportions[i] > 0.3)
		printf(COLOR_YELLOW);
		else if (p.proportions[i] > 0.1)
		printf(COLOR_GREEN);

		printf("%.3f", p.proportions[i]);
		printf(COLOR_NORMAL);
		printf("|");
	}
	printf("\n");
}

static void normalize(portfolio &p, int size)
{
	float sum = 0.f;
	for(int i = 0; i < size; i++) {
		if (p.proportions[i] < 0.f)
			p.proportions[i] = 0.f;
		sum += p.proportions[i];
	}

	for(int i = 0; i < size; i++)
		p.proportions[i] /= sum;
}

static void make_delta(portfolio &p, int size)
{
	for(int i = 0; i < size; i++) {
		int r = rand() % 3;
		switch(r) {
			case 0:
			p.proportions[rand()%size] = 1.01f;
			break;
			case 1:
			p.proportions[rand()%size] = 0.99f;
			break;
			case 2:
			p.proportions[rand()%size] = 1.f;
			break;
		}
	}
}

static void make_single_delta(portfolio &p, int index, int size, float factor)
{
	index %= size;

	for(int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	if (rand() % 2 == 0)
		p.proportions[index] = 1.0f * factor;
	else 
		p.proportions[index] = -1.0f * factor;
}

static void add(portfolio &p, portfolio& delta, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] += delta.proportions[i];
}

static void mul(portfolio &p, portfolio& delta, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] *= delta.proportions[i];
}

static float fitness_function(float expectancy, float standard_deviation)
{
	float factor1 = 0.f;
	float factor2 = 0.f;

	if (expectancy < 1.6f)
		factor1 = (1.6f - expectancy) * 1.9f;
	else
		factor1 = expectancy * -0.001f;

	if (standard_deviation > 0.35f)
		factor2 = standard_deviation - 0.35f;
	else
		factor2 = 0.001f * standard_deviation;

	return - factor1 - factor2;
}

void stochastic_optimization(std::vector<data_series> & historical_data)
{
	portfolio p, p_new;
	int iteration = 0;
	int size = historical_data.size();

	// Initialize our portfolio randomly
	randomize(p, size);
	normalize(p, size);

	float fitness = MINFLOAT;
	float expectancy, standard_deviation;

	monte_carlo m(historical_data, true);
	int num_rounds = 32768;
	m.run(p_new, expectancy, standard_deviation, num_rounds);
	fitness = fitness_function(expectancy, standard_deviation);
	portfolio delta;


	for(iteration = 0; iteration < 10 * size + 500; iteration++) {
		if (iteration < 10 * size) {
			static int first_iteration = iteration;
			float factor = 1.f - (float)(iteration - first_iteration)/(size * 10.f);
			make_single_delta(delta, iteration % size, size, factor);
		} else {
			make_delta(delta, size);
		}

do_it_again:
		if (iteration < 10 * size) {
			p_new = p;
			add (p_new, delta, size);
			normalize(p_new, size);
		} else {
			p_new = p;
			mul (p_new, delta, size);
			normalize(p_new, size);
		}

		m.run(p_new, expectancy, standard_deviation, num_rounds);
		float new_fitness = fitness_function(expectancy, standard_deviation);
		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f e = %f σ = %f \n", fitness, expectancy, standard_deviation);
			print_portfolio(p, historical_data, size);
			goto do_it_again;
		}
	}

	print_portfolio(p, historical_data, size);
	for(int l = 0; l < 10; l++) {
		m.run(p, expectancy, standard_deviation, num_rounds);
		printf("e = %f σ = %f \n", expectancy, standard_deviation);
	}
}

