#include "includes.h"
#include "neural_net.h"
#include "problem.h"
#include "util.h"

static const float threshold = 0.00000000001f;

static float error (problem & p, neural_net & n, float min_error = FLT_MAX)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float error;

	error = 0.f;
	for (int items = 0; items < min (20, p.size ()); items++) {
		p.expectations (input_data, desired_data);
		n.forward_propagation (input_data, output_data);
		for (int o = 0; o < p.num_outputs; o++)
			error += ((desired_data[o] - output_data[o]) * (desired_data[o] - output_data[o])) / 1000.f;
	}

	if (isinf (error) || isnan (error))
		return FLT_MAX;

	// return early if we are very bad
	if (error * 170.9f / 2.f > min_error)
		return error;

	for (int items = 20; items < p.size (); items++) {
		p.expectations (input_data, desired_data);
		n.forward_propagation (input_data, output_data);
		for (int o = 0; o < p.num_outputs; o++)
			error += ((desired_data[o] - output_data[o]) * (desired_data[o] - output_data[o])) / 1000.f;
	}

	if (isinf (error) || isnan (error))
		return FLT_MAX;

	return error;
}

static void maybe_printf (const char *format, ...)
{
	static time_t last_print;

	struct timeval tv;
	gettimeofday (&tv, NULL);

	if (tv.tv_sec == last_print)
		return;

	last_print = tv.tv_sec;

	clear_line ();

	va_list args;
	va_start (args, format);
	vprintf (format, args);
	va_end (args);

	fflush (stdout);
}

static void maybe_printn (neural_net & n)
{
	static time_t last_print = 0;

	struct timeval tv;
	gettimeofday (&tv, NULL);

	if (tv.tv_sec == last_print)
		return;

	last_print = tv.tv_sec;

	n.print ();
}

static void maybe_save (neural_net & n)
{
	static time_t last_save = 0;

	struct timeval tv;
	gettimeofday (&tv, NULL);

	// Save every minute at most
	if (tv.tv_sec < last_save + 5)
		return;

	last_save = tv.tv_sec;

	n.write_to_disk ();
}

void training_annealing (problem & p, neural_net & n)
{
	float current_error = error (p, n);
	int last_improvement = 0;
	float step = 1.0f;

	while (current_error > threshold) {
		n.stash_state ();
		printf ("Annealing phase. Error: %.10f\n", current_error);

		for (int iteration = 0; iteration < 2000; iteration++) {
			maybe_printf ("Annealing %d/2000 error: %.10f", iteration, current_error);
			n.simulated_annealing (step / (float) iteration);

			float new_error = error (p, n, current_error);
			if (new_error <= current_error) {
				clear_line ();
				printf ("Annealing phase improvement. Error: %.10f\n", new_error);

				current_error = new_error;
				n.stash_state ();
				last_improvement = 0;
			}
			else {
				n.backtrack_state ();
				last_improvement++;
			}
		}
		n.backtrack_state ();
		clear_line ();

		// If we stopped making progress, reduce the step size
		if (last_improvement > 2000) {
			step /= 10.f;
			last_improvement = 0;
			printf ("Reducing annealing step to %f\n", step);
		}

		maybe_save (n);
	}

	printf ("Training done, error is %.10f\n", current_error);
}

void training_adagrad (problem & p, neural_net & n)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float current_error = error (p, n);

	while (current_error > threshold) {
		printf ("Adagrad phase. Error: %.10f\n", current_error);
		for (int iteration = 0; iteration < p.size (); iteration++) {
			p.expectations (input_data, desired_data);
			n.adagrad (input_data, desired_data, output_data);
			maybe_printf ("Iteration %d", iteration);
		}
		clear_line ();

		current_error = error (p, n);

		maybe_save (n);
	}

	printf ("Training done, error is %.10f\n", current_error);
}

void training_adagrad_annealing (problem & p, neural_net & n)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float current_error = error (p, n);

	while (current_error > threshold) {
		n.stash_state ();
		printf ("Annealing phase. Error: %.10f\n", current_error);

		for (int iteration = 0; iteration < 200; iteration++) {
			maybe_printf ("Annealing %d/200 error: %.10f", iteration, current_error);
			n.simulated_annealing (1.f / (float) iteration);

			float new_error = error (p, n, current_error);
			if (new_error <= current_error) {
				current_error = new_error;
				n.stash_state ();
			}
			else
				n.backtrack_state ();
		}
		n.backtrack_state ();
		clear_line ();

		printf ("Adagrad phase. Error: %.10f\n", current_error);
		for (int iteration = 0; iteration < 5000; iteration++) {
			p.expectations (input_data, desired_data);
			n.adagrad (input_data, desired_data, output_data);
			maybe_printf ("Iteration %d/5000", iteration);
		}
		clear_line ();

		current_error = error (p, n);

		maybe_save (n);
	}

	printf ("Training done, error is %.10f\n", current_error);
}

void training_backprop_annealing (problem & p, neural_net & n)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float current_error = error (p, n);

	while (current_error > threshold) {
		n.stash_state ();
		printf ("Annealing phase. Error: %.10f\n", current_error);

		for (int iteration = 0; iteration < 200; iteration++) {
			maybe_printf ("Annealing %d/200 error: %.10f", iteration, current_error);
			n.simulated_annealing (1.f / (float) iteration);

			float new_error = error (p, n, current_error);
			if (new_error <= current_error) {
				current_error = new_error;
				n.stash_state ();
			}
			else
				n.backtrack_state ();
		}
		n.backtrack_state ();
		clear_line ();

		printf ("Back propagation phase. Error: %.10f\n", current_error);
		for (int iteration = 0; iteration < 5000; iteration++) {
			p.expectations (input_data, desired_data);
			n.back_propagation (input_data, desired_data, output_data);
			maybe_printf ("Iteration %d/5000", iteration);
		}
		clear_line ();

		current_error = error (p, n);

		maybe_save (n);

	}

	printf ("Training done, error is %.10f\n", current_error);
}

void training_backprop (problem & p, neural_net & n)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float current_error = error (p, n);

	while (current_error > threshold) {
		printf ("Back propagation phase. Error: %.10f\n", current_error);
		for (int iteration = 0; iteration < 5000; iteration++) {
			p.expectations (input_data, desired_data);
			n.back_propagation (input_data, desired_data, output_data);
			maybe_printf ("Iteration %d/5000", iteration);
		}
		clear_line ();

		current_error = error (p, n);

		maybe_save (n);
	}

	printf ("Training done, error is %.10f\n", current_error);
}

void training_prune (problem & p, neural_net & n)
{
	float input_data[p.num_inputs];
	float desired_data[p.num_outputs];
	float output_data[p.num_outputs];
	float current_error;

	int num_prunes = 16;

	// TODO: try to push link values in the graph to 0

	current_error = error (p, n);
	printf ("Pruning phase. Error : %f\n", current_error);
	for (int iteration = 0; iteration < 2000; iteration++) {
		printf ("PRUNES %d\n", num_prunes);
		maybe_printf ("Iteration %d/2000 links: %d error: %.10f", iteration, n.num_links (), current_error);
		n.cleanup ();
		n.stash_state ();
		for (int i = 0; i < num_prunes; i++)
			n.zero_link ();
		for (int i = 0; i < 50000; i++) {
			p.expectations (input_data, desired_data);
			n.back_propagation (input_data, desired_data, output_data);
			if ((i % 1000) == 999) {
				current_error = error (p, n);
				if (current_error <= threshold)
					break;
			}
		}
		if (current_error > threshold) {
			n.backtrack_state ();
			num_prunes = max (1, num_prunes / 2);
		}
		maybe_printn (n);
		maybe_save (n);
	}
	n.print ();
	printf ("Pruning done, final error %.10f\n", current_error);
}
