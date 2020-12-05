#include "includes.h"
#include "loader.h"
#include "monte_carlo.h"
#include "portfolio.h"
#include "stochastic_optimization.h"

int main(int argc, char* argv[])
{
	srand(time(NULL));
	char* read_filename, * write_filename;

	bool need_optimize = false, need_read = false, need_write = false, need_evaluate = false;

	for(int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-o"))
			need_optimize = true;
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

	if (need_optimize)
		stochastic_optimization(data);

	if (need_evaluate) {
		float expectancy, standard_deviation;
		monte_carlo m(data, true);
		int num_rounds = 32768 * 16;
		m.run(p, expectancy, standard_deviation, num_rounds);
		printf("Overall e = %f σ = %f \n", expectancy, standard_deviation);

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

