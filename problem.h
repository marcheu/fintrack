#ifndef _PROBLEM_H_
#define _PROBLEM_H_

#include "neural_net.h"

struct problem {
	int num_inputs;
	int num_outputs;
	int num_hidden_layers;
	int hidden_layer_neurons[100];

	const char *name;

	void (*expectations) (float *input, float *desired_output);
	void (*initialize) (const char *name);
	int (*size) ();

	void (*inference) (neural_net & n);
};

extern struct problem problem_prediction;

#endif
