#include "monte_carlo.h"
#include "stochastic_optimization.h"

static void make_delta (portfolio & p, int size)
{
	for (int i = 0; i < size; i++) {
		int r = rand () % 3;
		switch (r) {
		case 0:
			p.proportions[i] = 1.0f;
			break;
		case 1:
			p.proportions[i] = 0.8f;
			break;
		case 2:
			p.proportions[i] = 1.2f;
			break;
		}
	}
}

static void make_single_delta (portfolio & p, int index, int size, float factor)
{
	index %= size;

	for (int i = 0; i < size; i++)
		p.proportions[i] = 0.f;

	if (rand () % 2 == 0)
		p.proportions[index] = 1.0f * factor;
	else
		p.proportions[index] = -1.0f * factor;
}

static void add (portfolio & p, portfolio & delta, int size)
{
	for (int i = 0; i < size; i++)
		p.proportions[i] += delta.proportions[i];
}

static void mul (portfolio & p, portfolio & delta, int size)
{
	for (int i = 0; i < size; i++)
		p.proportions[i] *= delta.proportions[i];
}

static void one (portfolio & p, int size)
{
	for (int i = 0; i < size; i++)
		p.proportions[i] = 1.f;
}

static float fitness_function (std::vector < data_series > &data, portfolio & p, float expectancy, float downsize_75_deviation, float goal)
{
	float factor1 = 0.f;
	float factor2 = 0.f;
	float factor3 = 0.f;

	/* The fitness function that evaluates a portfolio has 3 main components:
	 *
	 * 1. We want to try to get at least the user-specific expectancy. Anything
	 * above the goal is only a small win, anything below the goal is bad.
	 *
	 * 2. We want to minimize the risk, i.e. the downside deviation.
	 *
	 * 3. We try to get some diversification, i.e. don't put more than 100% in a
	 * single sector (keep in mind we are leveraged so we can go to a total of 300%)
	 *
	 * We sum all 3 components into a single metric, which is the touchy part: how
	 * much weight should be given to each is very much a personal preference.
	 * These should be reasonable (?) defaults.
	 */

	// 1. Maximize expectancy
	if (expectancy < goal)
		factor1 = (goal - expectancy) * 5.0f;
	else
		factor1 = expectancy * -0.01f;

	// 2. Minimize downside 75 deviation
	factor2 = downsize_75_deviation * 50.f;

	// 3. Try to get some diversification
	float sectors[NUM_SECTORS], total = 0.f;
	for (unsigned s = 0; s < NUM_SECTORS; s++)
		sectors[s] = 0.f;

	for (unsigned t = 0; t < data.size (); t++) {
		for (unsigned s = 0; s < NUM_SECTORS; s++) {
			float v = p.proportions[t] * data[t].leverage * data[t].sector_proportions[s];
			sectors[s] += v;
			total += v;
		}
	}
	for (unsigned s = 0; s < NUM_SECTORS; s++)
		if (sectors[s] > 100.f)
			factor3 += (sectors[s] - 100.f);
	factor3 /= 100.f;

	return -factor1 - factor2 - factor3;
}

void stochastic_optimization (std::vector < data_series > &historical_data, portfolio & p, bool refine, int days_back, float goal)
{
	portfolio p_new;
	portfolio delta;
	int iteration = 0;
	int size = historical_data.size ();
	float fitness = -FLT_MAX;
	float expectancy, standard_deviation, downside_deviation, downsize_75_deviation;
	int num_rounds = 1 << 16;
	int stagnate = 0;

	monte_carlo m (historical_data, true);

	p.print (historical_data);
	m.run (p, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
	fitness = fitness_function (historical_data, p, expectancy, downsize_75_deviation, goal);


	if (!refine) {

		// Initialize with ones
		one (p, size);
		p.normalize ();

		// Coarse pass
		m.run (p, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
		fitness = fitness_function (historical_data, p, expectancy, downsize_75_deviation, goal);

		float factor = 0.5f;
		while (stagnate < 2000) {
			make_single_delta (delta, iteration % size, size, factor);

		      do_it_again:
			p_new = p;
			add (p_new, delta, size);
			p_new.normalize ();
			p_new.max_proportions (historical_data);

			m.run (p_new, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
			float new_fitness = fitness_function (historical_data, p, expectancy, downsize_75_deviation, goal);
			if (new_fitness > fitness) {
				p = p_new;
				fitness = new_fitness;
				printf ("fitness now %f e = %f σ = %f σd = %f σd75 = %f \n", fitness, expectancy, standard_deviation, downside_deviation, downsize_75_deviation);
				stagnate = 0;
				p.print (historical_data);
				goto do_it_again;
			}
			iteration++;

			stagnate++;
			if ((stagnate == 2000) && factor > 0.1f) {
				factor /= 1.5f;
				stagnate = 0;
				printf ("new factor %f\n", factor);
			}
		}

		printf ("============================\n");
	}

	// Fine pass
	stagnate = 0;
	num_rounds = 1 << 16;
	while (stagnate < 10000) {
		make_delta (delta, size);

	      do_it_again2:
		p_new = p;
		mul (p_new, delta, size);
		p_new.normalize ();
		p_new.max_proportions (historical_data);

		m.run (p_new, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
		float new_fitness = fitness_function (historical_data, p, expectancy, downsize_75_deviation, goal);
		if (new_fitness > fitness) {
			stagnate = 0;
			p = p_new;
			fitness = new_fitness;
			printf ("fitness now %f e = %f σ = %f σd = %f σd75 = %f \n", fitness, expectancy, standard_deviation, downside_deviation, downsize_75_deviation);
			p.print (historical_data);
			goto do_it_again2;
		}
		iteration++;

		stagnate++;

		if ((stagnate == 5000) && (num_rounds < 1 << 17)) {
			num_rounds *= 2;
			stagnate = 0;
			printf ("rounds %d\n", num_rounds);
			m.run (p, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
			fitness = fitness_function (historical_data, p, expectancy, downsize_75_deviation, goal);
		}
	}

	p.print_as_file (historical_data);
	for (int l = 0; l < 10; l++) {
		m.run (p, expectancy, standard_deviation, downside_deviation, downsize_75_deviation, num_rounds, days_back);
		printf ("e = %f σ = %f σd = %f σd75 = %f \n", expectancy, standard_deviation, downside_deviation, downsize_75_deviation);
	}
}
