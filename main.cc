#include "backtest.h"
#include "constants.h"
#include "evaluation.h"
#include "includes.h"
#include "loader.h"
#include "make_3x.h"
#include "monte_carlo.h"
#include "neural_net.h"
#include "portfolio.h"
#include "problem.h"
#include "stochastic_optimization.h"
#include "tests.h"
#include "training.h"
#include "util.h"

int main (int argc, char *argv[])
{
	srand (time (NULL));
	char *read_filename = NULL, *write_filename = NULL, *make_filename = NULL;
	float goal = 1.65f;	// default 65% target gain per year

	bool need_optimize = false, need_learn = false, need_read = false, need_write = false, need_evaluate = false, need_backtest = false, need_test = false, need_stocks = false, need_make_3x = false;

	for (int i = 1; i < argc; i++) {
		if (!strcmp (argv[i], "-o"))
			need_optimize = true;
		else if (!strcmp (argv[i], "-l"))
			need_learn = true;
		else if (!strcmp (argv[i], "-s"))
			need_stocks = true;
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
		else if (!strcmp (argv[i], "-b"))
			need_backtest = true;
		else if (!strcmp (argv[i], "-e"))
			need_evaluate = true;
		else if (!strcmp (argv[i], "-t"))
			need_test = true;
		else if (!strcmp (argv[i], "-g")) {
			i++;
			goal = strtof (argv[i], NULL);
		}
		else if (!strcmp (argv[i], "-m")) {
			need_make_3x = true;
			i++;
			make_filename = argv[i];
		}
		else if (!strcmp (argv[i], "-n")) {
			char output_file[1024] = "nets/";
			i++;
			strcat (output_file, argv[i]);
			problem p = problem_prediction;
			p.initialize (argv[i]);
			neural_net n (p.num_inputs, p.num_outputs, p.num_hidden_layers, p.hidden_layer_neurons, output_file);
			n.read_from_disk ();
			training_backprop (p, n);
		}
		else if (!strcmp (argv[i], "-i")) {
			char output_file[1024] = "nets/";
			i++;
			strcat (output_file, argv[i]);
			problem p = problem_prediction;
			p.initialize (argv[i]);
			neural_net n (p.num_inputs, p.num_outputs, p.num_hidden_layers, p.hidden_layer_neurons, NULL);
			int r = n.read_from_disk ();
			assert (r);
			for (int i = 0; i < 1000; i++)
				p.inference (n);
		}
	}

	std::vector < data_series > data;
	loader l;
	l.load_all_series (data, need_stocks, false);

	portfolio p;
	if (need_read) {
		p.read (read_filename, data);
		p.normalize ();
		p.print (data);
	}
	else {
		p.constant (data);
		p.normalize ();
	}

	if (need_test)
		run_tests ();

	if (need_learn)
		stochastic_optimization (data, p, false, TRADING_DAYS_PER_YEAR * 10, goal);

	if (need_optimize)
		stochastic_optimization (data, p, true, TRADING_DAYS_PER_YEAR * 10, goal);

	if (need_evaluate)
		evaluate_portfolio (data, p);

	if (need_backtest)
		backtest_portfolio (data, p);

	if (need_make_3x)
		make_3x (data, p, make_filename);

	if (need_write)
		p.write (write_filename, data);

	return 0;
}
