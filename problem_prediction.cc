#include "data_series.h"
#include "loader.h"
#include "neural_net.h"
#include "problem.h"

static std::vector < data_series > training_data;
static int vix_index = -1;
static int predicted_stock = -1;

static int stock_index (const char *name)
{
	unsigned i;
	for (i = 0; i < training_data.size (); i++)
		if (!strcmp (training_data[i].name, name)) {
			break;
		}
	assert (i < training_data.size ());
	return i;
}

static void prediction_initialize (const char *name)
{
	loader l;
	l.load_all_series (training_data, false, false);

	vix_index = stock_index ("^VIX");
	predicted_stock = stock_index (name);
}

static int prediction_size ()
{
	return 10000;
}

static float average_value (int stock, int index, int dim)
{
	float sum = 0.f;
	for (int i = 1; i <= dim; i++)
		sum += training_data[stock].values[index - i];

	sum /= training_data[stock].values[index - 1] * (float) dim;

	return sum;
}

static void prepare_inputs (float *input, int stock, int index, int day_of_week)
{
	// 5 entries for possible days of the week
	input[0] = (float) (day_of_week == 0);
	input[1] = (float) (day_of_week == 1);
	input[2] = (float) (day_of_week == 2);
	input[3] = (float) (day_of_week == 3);
	input[4] = (float) (day_of_week == 4);

	// 7 entries stock history
	input[5] = training_data[stock].values[index - 1] / training_data[stock].values[index];
	input[6] = training_data[stock].values[index - 2] / training_data[stock].values[index];
	input[7] = training_data[stock].values[index - 3] / training_data[stock].values[index];
	input[8] = training_data[stock].values[index - 4] / training_data[stock].values[index];
	input[9] = training_data[stock].values[index - 5] / training_data[stock].values[index];
	input[10] = training_data[stock].values[index - 6] / training_data[stock].values[index];
	input[11] = training_data[stock].values[index - 7] / training_data[stock].values[index];

	// 1 entry for current volatility
	input[12] = training_data[vix_index].values[index - 1] / 1000.f;

	// 7 entries averagees
	input[13] = average_value (stock, index, 8);
	input[14] = average_value (stock, index, 12);
	input[15] = average_value (stock, index, 16);
	input[16] = average_value (stock, index, 20);
	input[17] = average_value (stock, index, 24);
	input[18] = average_value (stock, index, 28);
	input[19] = average_value (stock, index, 32);

}

static void prediction_expectations (float *input, float *output)
{
	assert (training_data[predicted_stock].size > 75);

	int index = rand () % (training_data[predicted_stock].size - 75) + 35;
	assert (index - 34 >= 0);
	assert (index + 40 < training_data[predicted_stock].size);
	date d = training_data[predicted_stock].dates[index];

	int day_of_week = d.day_of_the_week ();

	// Only trade Monday-Friday
	assert (day_of_week >= 0);
	assert (day_of_week <= 4);

	prepare_inputs (input, predicted_stock, index, day_of_week);

	output[0] = training_data[predicted_stock].values[index + 40] / training_data[predicted_stock].values[index];
}

static void prediction_inference (neural_net & n)
{
	float input[20];
	float output[4];

	int index = training_data[predicted_stock].size - 1;
	assert (index - 34 >= 0);
	assert (index + 40 < training_data[predicted_stock].size);
	date d = training_data[predicted_stock].dates[index];
	int day_of_week = d.day_of_the_week ();

	prepare_inputs (input, predicted_stock, index, day_of_week);

	n.forward_propagation (input, output);

	printf ("%f\n", output[0]);
}

struct problem problem_prediction = {
	.num_inputs = 20,	// 5 possible days, 7 history, 7 volatility
	.num_outputs = 1,	// predicted values
	.num_hidden_layers = 2,
	.hidden_layer_neurons = {40, 30},

	.name = "Prediction",

	.expectations = prediction_expectations,
	.initialize = prediction_initialize,
	.size = prediction_size,

	.inference = prediction_inference
};
