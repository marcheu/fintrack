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
	for(int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	if ((rand()%2) == 0)
		p.proportions[rand()%size] = 0.05f;
	else
		p.proportions[rand()%size] = -0.05f;
}

static void make_single_delta(portfolio &p, int index, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	p.proportions[index] = 1.0f;
}

static void make_negative_delta(portfolio &p, int index, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	p.proportions[index] = -1.0f;
}

static void shrink_delta(portfolio &p, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] = p.proportions[i] * 0.6f;
}


static void add(portfolio &p, portfolio& delta, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] += delta.proportions[i];
}

static float fitness_function(float expectancy, float standard_deviation)
{
	float factor1 = 0.f;
	float factor2 = 0.f;

	if (expectancy < 1.4f)
		factor1 = (1.4f - expectancy) * 3;

	if (standard_deviation > 0.4f)
		factor2 = standard_deviation - 0.4f;

	return expectancy /*- standard_deviation / 1.8f*/ - factor1 - factor2;
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

	monte_carlo m(historical_data);

	// initialization pass
	while(iteration < 10) {
		randomize(p_new, size);
		normalize(p_new, size);
		m.run(p_new, expectancy, standard_deviation);
		float new_fitness = fitness_function(expectancy, standard_deviation);
		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			print_portfolio(p, historical_data, size);
		}
		iteration ++;
	}

	// single vector up pass
	iteration = 0;
	while(iteration < size) {
		portfolio delta;
		make_single_delta(delta, iteration, size);

do_it_again:

		p_new = p;
		add (p_new, delta, size);
		normalize(p_new, size);

		m.run(p_new, expectancy, standard_deviation);

		float new_fitness = fitness_function(expectancy, standard_deviation);

		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			print_portfolio(p, historical_data, size);
			shrink_delta(delta, size);
			goto do_it_again;
		}

		iteration ++;
	}

	// single vector down pass
	iteration = 0;
	while(iteration < size) {
		portfolio delta;
		make_negative_delta(delta, iteration, size);

do_it_again_neg:

		p_new = p;
		add (p_new, delta, size);
		normalize(p_new, size);

		m.run(p_new, expectancy, standard_deviation);

		float new_fitness = fitness_function(expectancy, standard_deviation);

		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			print_portfolio(p, historical_data, size);
			shrink_delta(delta, size);
			goto do_it_again_neg;
		}

		iteration ++;
	}

	// real pass
	while(iteration < 400) {
		portfolio delta;
		make_delta(delta, size);

do_it_again2:

		p_new = p;
		add (p_new, delta, size);
		normalize(p_new, size);

		m.run(p_new, expectancy, standard_deviation);

		float new_fitness = fitness_function(expectancy, standard_deviation);

		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			print_portfolio(p, historical_data, size);
			shrink_delta(delta, size);
			goto do_it_again2;
		}

		iteration ++;

//		printf("iteration %d done\n",iteration);
	}
	m.run(p, expectancy, standard_deviation);
	print_portfolio(p, historical_data, size);
	printf("exp: %f\n",expectancy);
	printf("stddev: %f\n",standard_deviation);
}

