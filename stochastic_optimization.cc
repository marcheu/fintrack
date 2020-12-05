#include "monte_carlo.h"
#include "stochastic_optimization.h"

static void make_delta(portfolio &p, int size)
{
	for(int i = 0; i < size; i++) {
		int r = rand() % 3;
		switch(r) {
			case 0:
			p.proportions[i] = 1.0f;
			break;
			case 1:
			p.proportions[i] = 0.99f;
			break;
			case 2:
			p.proportions[i] = 1.01f;
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

	if (expectancy < 1.45f)
		factor1 = (1.45f - expectancy) * 1.9f;
	else
		factor1 = expectancy * -0.1f;

	if (standard_deviation > 0.35f)
		factor2 = standard_deviation - 0.35f;
	else
		factor2 = 0.1f * standard_deviation;

	return - factor1 - factor2;
/*
	factor1 = 4.0f *fabs(expectancy - 1.6f) ;
	factor2 = 0.65f * standard_deviation;
	return - factor1 - factor2;*/
}

void stochastic_optimization(std::vector<data_series> & historical_data)
{
	portfolio p, p_new;
	int iteration = 0;
	int size = historical_data.size();

	// Initialize our portfolio randomly
	p.randomize(historical_data);
	p.normalize();

	float fitness = MINFLOAT;
	float expectancy, standard_deviation;

	monte_carlo m(historical_data, true);


	// Coarse pass
	int num_rounds = 4096;
	m.run(p, expectancy, standard_deviation, num_rounds);
	fitness = fitness_function(expectancy, standard_deviation);
	portfolio delta;

	int stagnate = 0;
	float factor = 0.5f;
	while(stagnate < 2000) {
		make_single_delta(delta, iteration % size, size, factor);

do_it_again:
		p_new = p;
		add (p_new, delta, size);
		p_new.normalize();

		m.run(p_new, expectancy, standard_deviation, num_rounds);
		float new_fitness = fitness_function(expectancy, standard_deviation);
		if (new_fitness > fitness) {
			stagnate = 0;
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f e = %f σ = %f \n", fitness, expectancy, standard_deviation);
			p.print(historical_data);
			goto do_it_again;
		}
		iteration++;

		stagnate++;
		if ((stagnate == 2000) && factor > 0.001f) {
			factor /= 2.f;
			stagnate = 0;
		}
	}

	printf("============================\n");

	// Fine pass
	stagnate = 0;
	while(stagnate < 2000) {
		make_delta(delta, size);

do_it_again2:
		p_new = p;
		mul (p_new, delta, size);
		p_new.normalize();

		m.run(p_new, expectancy, standard_deviation, num_rounds);
		float new_fitness = fitness_function(expectancy, standard_deviation);
		if (new_fitness > fitness) {
			stagnate = 0;
			p = p_new;
			fitness = new_fitness;
			printf("fitness now %f e = %f σ = %f \n", fitness, expectancy, standard_deviation);
			p.print(historical_data);
			goto do_it_again2;
		}
		iteration++;

		stagnate++;

		if ((stagnate == 2000) && (num_rounds < 32768)) {
			num_rounds *= 4;
			stagnate = 0;
			printf("rounds %d\n",num_rounds);
			m.run(p, expectancy, standard_deviation, num_rounds);
			fitness = fitness_function(expectancy, standard_deviation);
		}
	}

	p.print(historical_data);
	for(int l = 0; l < 10; l++) {
		m.run(p, expectancy, standard_deviation, num_rounds);
		printf("e = %f σ = %f \n", expectancy, standard_deviation);
	}
}

