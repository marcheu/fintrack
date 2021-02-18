#include "neural_net.h"

neural_net::neural_net (int num_input_neurons, int num_output_neurons, int num_hidden_layers, int num_hidden_neurons[], char *file_name)
{
	assert (num_input_neurons > 0);
	assert (num_hidden_layers > 0);
	assert (num_hidden_neurons > 0);
	assert (num_output_neurons > 0);

	num_input_neurons_ = num_input_neurons;
	num_output_neurons_ = num_output_neurons;
	memcpy (num_neurons_per_layer_, num_hidden_neurons, sizeof (int) * MAX_HIDDEN_LAYERS);
	num_hidden_layers_ = num_hidden_layers;

	index_input_ = 0;
	index_bias_ = num_input_neurons_;
	int sum = index_bias_ + 1;
	for (int i = 0; i < num_hidden_layers_; i++) {
		index_hidden_[i] = sum;
		sum += num_neurons_per_layer_[i];
	}
	index_output_ = sum;
	num_neurons_ = index_output_ + num_output_neurons;
	num_links_ = num_neurons_ * num_neurons_;


	graph_ = (bool *) malloc (num_links_ * sizeof (bool));
	graph_save_ = (bool *) malloc (num_links_ * sizeof (bool));
	weights_ = (float *) malloc (num_links_ * sizeof (float));
	weights_save_ = (float *) malloc (num_links_ * sizeof (float));
	steps_ = (float *) malloc (num_links_ * sizeof (float));
	values_ = (float *) malloc (num_neurons_ * sizeof (float));
	errors_ = (float *) malloc (num_neurons_ * sizeof (float));

	// build the net

	// initialize neurons
	for (int i = 0; i < num_neurons_; i++) {
		values_[i] = 0.0f;
		errors_[i] = 0.0f;
	}
	values_[neuron_index (type_bias, 0, 0)] = 1.0f;

	// initialize links
	for (int i = 0; i < num_links_; i++) {
		graph_[i] = false;
		graph_save_[i] = false;
		weights_[i] = 0.0f;
		weights_save_[i] = 0.0f;
		steps_[i] = 0.000001f;
	}

	// link all input neurons to the first hidden layer
	for (int i = 0; i < num_input_neurons_; i++) {
		for (int h = 0; h < num_neurons_per_layer_[0]; h++) {
			int ii = neuron_index (type_input, 0, i);
			int ih = neuron_index (type_hidden, 0, h);
			create_link (ii, ih);
			int ib = neuron_index (type_bias, 0, 0);
			create_link (ib, ih);
		}
	}

	// for each hidden layer
	for (int l = 0; l < num_hidden_layers_ - 1; l++) {
		for (int h1 = 0; h1 < num_neurons_per_layer_[l]; h1++) {
			for (int h2 = 0; h2 < num_neurons_per_layer_[l + 1]; h2++) {
				int ih1 = neuron_index (type_hidden, l, h1);
				int ih2 = neuron_index (type_hidden, l + 1, h2);
				create_link (ih1, ih2);
				int ib = neuron_index (type_bias, 0, 0);
				create_link (ib, ih2);
			}
		}
	}

	// for the last hidden to the output layer
	for (int h = 0; h < num_neurons_per_layer_[num_hidden_layers - 1]; h++) {
		for (int o = 0; o < num_output_neurons_; o++) {
			int ih = neuron_index (type_hidden, num_hidden_layers_ - 1, h);
			int io = neuron_index (type_output, 0, o);
			create_link (ih, io);
			int ib = neuron_index (type_bias, 0, 0);
			create_link (ib, io);
		}
	}

	output_file_ = strdup (file_name);
}

void neural_net::forward_propagation (float *input_values, float *output_values)
{
	for (int i = 0; i < num_input_neurons_; i++) {
		values_[index_input_ + i] = activation_function (input_values[i]);
	}

	float sum;
	for (int d = index_hidden_[0]; d < num_neurons_; d++) {
		sum = 0.f;
		for (int s = 0; s < d; s++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;

			sum += weights_[i] * values_[s];
		}
		values_[d] = activation_function (sum);
	}

	for (int o = 0; o < num_output_neurons_; o++) {
		output_values[o] = values_[index_output_ + o];
	}
}

void neural_net::back_propagation (float *input_values, float *actual_output_values, float *output_values)
{
	float sum;

	// first do a normal forward propagation
	forward_propagation (input_values, output_values);

	// compute the errors for the output layer
	for (int o = 0; o < num_output_neurons_; o++) {
		int io = index_output_ + o;
		float ov = values_[io];
		float av = actual_output_values[o];
		errors_[io] = (ov - av) * activation_function_derivative (values_[io]);
	}

	// compute the errors for the rest of the net
	for (int s = index_output_ - 1; s >= 0; s--) {
		sum = 0.f;
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;
			sum += errors_[d] * weights_[i];
		}

		errors_[s] = sum * activation_function_derivative (values_[s]);
	}

	// change the weights based on the error
	for (int s = index_input_; s < num_neurons_ - 1; s++) {
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;
			weights_[i] = weights_[i] - back_propagation_step_size_ * errors_[d] * values_[s];
		}
	}
}

void neural_net::adagrad (float *input_values, float *actual_output_values, float *output_values)
{
	float sum;

	// first do a normal forward propagation
	forward_propagation (input_values, output_values);

	// compute the errors for the output layer
	for (int o = 0; o < num_output_neurons_; o++) {
		int io = index_output_ + o;
		float ov = values_[io];
		float av = actual_output_values[o];
		errors_[io] = (ov - av) * activation_function_derivative (values_[io]);
	}

	// compute the errors for the rest of the net
	for (int s = index_output_ - 1; s >= 0; s--) {
		sum = 0.f;
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;
			sum += errors_[d] * weights_[i];
		}

		errors_[s] = sum * activation_function_derivative (values_[s]);
	}

	// change the weights based on the error
	for (int s = index_input_; s < num_neurons_ - 1; s++) {
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;
			weights_[i] = weights_[i] - adagrad_step_size_ / sqrtf (steps_[i]) * errors_[d] * values_[s];
			steps_[i] += errors_[d] * errors_[d];
		}
	}
}

void neural_net::simulated_annealing (float factor)
{
	float step = annealing_step_size_ * factor;

	for (int mutations = 0; mutations < num_links_ / 10; mutations++) {
		int index;

		do {
			index = rand () % num_links_;
		} while (!graph_[index]);

		if (rand () % 2)
			weights_[index] += step;
		else
			weights_[index] -= step;
	}
}

void neural_net::stash_state ()
{
	memcpy (weights_save_, weights_, sizeof (float) * num_links_);
	memcpy (graph_save_, graph_, sizeof (bool) * num_links_);
}

void neural_net::backtrack_state ()
{
	memcpy (weights_, weights_save_, sizeof (float) * num_links_);
	memcpy (graph_, graph_save_, sizeof (bool) * num_links_);
}

bool neural_net::neuron_has_source (int n)
{
	bool has_source = false;
	for (int s = 0; s < n; s++) {
		int i = link_index (s, n);
		if (!graph_[i])
			continue;

		has_source = true;
		break;
	}

	return has_source;
}

bool neural_net::neuron_has_destination (int n)
{
	bool has_dest = false;
	for (int d = n + 1; d < num_neurons_; d++) {
		int i = link_index (n, d);
		if (!graph_[i])
			continue;

		has_dest = true;
		break;
	}

	return has_dest;
}

bool neural_net::neuron_is_dead (int n)
{
	bool has_source = neuron_has_source (n);
	bool has_dest = neuron_has_destination (n);

	return !has_dest && !has_source;
}

void neural_net::remove_neuron (int n)
{
	// remove all links going from s to n
	for (int s = 0; s < n; s++) {
		int i = link_index (s, n);
		if (graph_[i])
			graph_[i] = false;
	}

	// remove all links going from n to d
	for (int d = n + 1; d < num_neurons_; d++) {
		int i = link_index (n, d);
		if (graph_[i])
			graph_[i] = false;
	}
}

void neural_net::cleanup ()
{
	bool has_source;
	bool has_dest;

	for (int d = index_hidden_[0]; d < index_output_; d++) {
		has_source = false;
		for (int s = 0; s < d; s++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;

			has_source = true;
			break;
		}
		if (!has_source)
			remove_neuron (d);
	}

	for (int s = index_hidden_[0]; s < index_output_; s++) {
		has_dest = false;
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				continue;

			has_dest = true;
			break;
		}
		if (!has_dest)
			remove_neuron (s);
	}
}

void neural_net::zero_link ()
{
	int i, min_index;

	for (i = 0; i < num_links_; i++)
		if (graph_[i])
			break;

	min_index = i;
	for (; i < num_links_; i++)
		if (graph_[i] && (fabs (weights_[i]) < fabs (weights_[min_index])) && (rand () % 10 < 5))
			min_index = i;

	weights_[min_index] = 0.0f;
	graph_[min_index] = false;
}

void neural_net::print ()
{
	int counted_links = 0;
	for (int i = 0; i < num_links_; i++)
		if (graph_[i])
			counted_links++;

	printf ("NEURONS: %d  LINKS: %d\n", num_neurons_, counted_links);

	if (num_neurons_ > 60) {
		printf ("Neural net too big, not printing\n");
		return;
	}

	printf ("      |  err     |   value  | ");
	for (int d = 0; d < num_neurons_; d++)
		printf ("   % 3d    |", d);
	printf ("\n");

	for (int s = 0; s < num_neurons_; s++) {
		if (s < index_bias_)
			printf (" I");
		else if (s < index_hidden_[0])
			printf (" B");
		else if (s < index_output_)
			printf (" H");
		else
			printf (" O");
		printf ("% 3d | ", s);
		printf ("%+.5f | ", errors_[s]);
		printf ("%+.5f | ", values_[s]);
		for (int d = 0; d < s + 1; d++)
			printf ("          |");
		for (int d = s + 1; d < num_neurons_; d++) {
			int i = link_index (s, d);
			if (!graph_[i])
				printf ("    xxx   |");
			else {
				printf ("%+8.4f  |", weights_[i]);
			}
		}
		printf ("\n");
	}
}

int neural_net::num_links ()
{
	int count = 0;
	for (int i = 0; i < num_links_; i++)
		if (graph_[i])
			count++;
	return count;
}

void neural_net::write_to_disk ()
{
	FILE *f = fopen (output_file_, "wb");

	fwrite (&num_neurons_, sizeof (num_neurons_), 1, f);
	fwrite (&num_input_neurons_, sizeof (num_input_neurons_), 1, f);
	fwrite (&num_output_neurons_, sizeof (num_output_neurons_), 1, f);
	fwrite (&num_hidden_layers_, sizeof (num_hidden_layers_), 1, f);
	fwrite (&num_neurons_per_layer_, sizeof (num_neurons_per_layer_), 1, f);

	fwrite (&index_input_, sizeof (index_input_), 1, f);
	fwrite (&index_bias_, sizeof (index_bias_), 1, f);
	fwrite (&index_hidden_, sizeof (index_hidden_), 1, f);
	fwrite (&index_output_, sizeof (index_output_), 1, f);

	fwrite (&num_links_, sizeof (num_links_), 1, f);
	fwrite (graph_, num_links_ * sizeof (bool), 1, f);
	fwrite (weights_, num_links_ * sizeof (float), 1, f);
	fwrite (steps_, num_links_ * sizeof (float), 1, f);

	fclose (f);
}

int neural_net::read_from_disk ()
{
	FILE *f = fopen (output_file_, "rb");
	int ret = 1;

	ret *= fread (&num_neurons_, sizeof (num_neurons_), 1, f);
	ret *= fread (&num_input_neurons_, sizeof (num_input_neurons_), 1, f);
	ret *= fread (&num_output_neurons_, sizeof (num_output_neurons_), 1, f);
	ret *= fread (&num_hidden_layers_, sizeof (num_hidden_layers_), 1, f);
	ret *= fread (&num_neurons_per_layer_, sizeof (num_neurons_per_layer_), 1, f);

	ret *= fread (&index_input_, sizeof (index_input_), 1, f);
	ret *= fread (&index_bias_, sizeof (index_bias_), 1, f);
	ret *= fread (&index_hidden_, sizeof (index_hidden_), 1, f);
	ret *= fread (&index_output_, sizeof (index_output_), 1, f);

	ret *= fread (&num_links_, sizeof (num_links_), 1, f);
	ret *= fread (graph_, num_links_ * sizeof (bool), 1, f);
	ret *= fread (weights_, num_links_ * sizeof (float), 1, f);
	ret *= fread (steps_, num_links_ * sizeof (float), 1, f);

	fclose (f);

	return ret;
}
