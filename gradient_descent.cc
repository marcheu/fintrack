#include "gradient_descent.h"
#include "monte_carlo.h"

static void randomize(portfolio &p, int size)
{
	for(int i = 0; i < size; i++) {
		p.proportions[i] = (float)(rand()%100);
	}
}

static void print(portfolio &p, std::vector<data_series> & historical_data, int size)
{
	for(int i = 0; i < size; i++)
		printf("%d: %s %f\n",i, historical_data[i].name, p.proportions[i]);
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

	p.proportions[rand()%size] = 0.1f;
	p.proportions[rand()%size] = -0.1f;
}

static void make_single_delta(portfolio &p, int index, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	p.proportions[index] = 1.0f;
}

static void shrink_delta(portfolio &p, int size)
{
	for(int i = 0; i < size; i++)
		p.proportions[i] = p.proportions[i] * 0.8f;
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

	if (expectancy < 1.6f)
		factor1 = 1.6f - expectancy;

	if (standard_deviation > 0.4f)
		factor2 = standard_deviation - 0.4f;

	return expectancy - standard_deviation / 1.8f - factor1 * factor1 - factor2 * factor2;
}

void gradient_descent(std::vector<data_series> & historical_data)
{
	portfolio p, p_new;
	int iteration = 0;
	int size = historical_data.size();

	printf("SIZE is %d\n",size);
	// Initialize our portfolio randomly
	randomize(p, size);
	normalize(p, size);

	float fitness = MINFLOAT;
	float expectancy, standard_deviation;

	while(iteration < 10) {
		randomize(p, size);
		normalize(p, size);
		monte_carlo(historical_data, p, expectancy, standard_deviation);
		float new_fitness = fitness_function(expectancy, standard_deviation);
		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
		}
		iteration ++;
		printf("Init iteration %d done\n",iteration);
	}

	for(int all=0; all< 2;all++) {
	iteration = 0;
	while(iteration < size) {
		portfolio delta;
		make_single_delta(delta, iteration, size);

do_it_again:

		p_new = p;
		add (p_new, delta, size);
		normalize(p_new, size);

		monte_carlo(historical_data, p_new, expectancy, standard_deviation);

		float new_fitness = fitness_function(expectancy, standard_deviation);

		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			shrink_delta(delta, size);
			goto do_it_again;
		}

		iteration ++;

		printf("iteration %d done\n",iteration);
	}
	}

	while(iteration < 70) {
		portfolio delta;
		make_delta(delta, size);

do_it_again2:

		p_new = p;
		add (p_new, delta, size);
		normalize(p_new, size);

		monte_carlo(historical_data, p_new, expectancy, standard_deviation);

		float new_fitness = fitness_function(expectancy, standard_deviation);

		if (new_fitness > fitness) {
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f\n", fitness);
			shrink_delta(delta, size);
			goto do_it_again2;
		}

		iteration ++;

		printf("iteration %d done\n",iteration);
	}
	print(p, historical_data, size);
	printf("exp: %f\n",expectancy);
	printf("stddev: %f\n",standard_deviation);
}

