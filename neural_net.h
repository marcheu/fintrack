#ifndef _NEURAL_NET_H_
#define _NEURAL_NET_H_

#include "includes.h"
#include "util.h"

#define RELU 1

#define MAX_HIDDEN_LAYERS 100

class neural_net {
      public:
	neural_net (int num_input_neurons, int num_output_neurons, int num_hidden_layers, int *num_hidden_neurons);

	// Inference
	void forward_propagation (float *input_values, float *output_values);

	// Training methods
	void back_propagation (float *input_values, float *actual_output_values, float *output_values);
	void adagrad (float *input_values, float *actual_output_values, float *output_values);
	void simulated_annealing (float factor);
	void zero_link ();
	void cleanup ();

	void stash_state ();
	void backtrack_state ();

	// Utilities
	void print ();
	int num_links ();

	void write_to_disk ();
	void read_from_disk ();

      private:
	int num_neurons_;
	int num_input_neurons_;
	int num_output_neurons_;
	int num_hidden_layers_;
	int num_neurons_per_layer_[MAX_HIDDEN_LAYERS];

	int index_input_;
	int index_bias_;
	int index_hidden_[MAX_HIDDEN_LAYERS];
	int index_output_;

	int num_links_;
	bool *graph_;
	bool *graph_save_;
	float *weights_;
	float *weights_save_;
	float *steps_;
	float *values_;
	float *errors_;


	enum neuron_type {
		type_input,
		type_bias,
		type_hidden,
		type_output
	};

	int neuron_index (int type, int layer, int num) {
		switch (type) {
		case type_input:
			assert (layer == 0);
			assert (num < num_input_neurons_);
			return index_input_ + num;
			case type_bias:assert (layer == 0);
			assert (num == 0);
			return index_bias_;
			case type_hidden:assert (num < num_neurons_per_layer_[layer]);
			return index_hidden_[layer] + num;
			case type_output:assert (layer == 0);
			assert (num < num_output_neurons_);
			return index_output_ + num;
			default:assert (false);
			return 0;
		}
	}
	int link_index (int n1, int n2) {
		assert (n1 < n2);
		int index = n1 * num_neurons_ + n2;
		assert (index >= 0);
		assert (index < num_links_);
		return index;
	}

	void create_link (int n1, int n2) {
		int index = link_index (n1, n2);
		graph_[index] = true;
		weights_[index] = frand () / 100.f;
	}

	float activation_function (float x) {
#ifdef RELU
		if (x < 0.0f)
			return x * 0.01f;
		return x;
#else
		return (1 / (1 + expf (-x)));
#endif
	}

	float activation_function_derivative (float x) {
#ifdef RELU
		if (x < 0.0f)
			return 0.01f;
		return 1.f;
#else
		return x * (1.f - x);
#endif
	}

	void remove_neuron (int n);
	bool neuron_is_dead (int n);
	bool neuron_has_source (int n);
	bool neuron_has_destination (int n);


	const float back_propagation_step_size_ = 0.001f;
	const float adagrad_step_size_ = 0.001f;
	const float annealing_step_size_ = 0.01f;

	// Disallow copy/assignment
	neural_net (const neural_net &);
	neural_net & operator= (const neural_net &);
};

#endif
