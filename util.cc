#include "includes.h"
#include "util.h"

static void print_char (float v)
{
	if (v >= 8.f / 8.f)
		printf ("█");
	else if (v >= 7.f / 8.f)
		printf ("▉");
	else if (v >= 6.f / 8.f)
		printf ("▊");
	else if (v >= 5.f / 8.f)
		printf ("▋");
	else if (v >= 4.f / 8.f)
		printf ("▌");
	else if (v >= 3.f / 8.f)
		printf ("▍");
	else if (v >= 2.f / 8.f)
		printf ("▎");
	else if (v >= 1.f / 8.f)
		printf ("▏");
	else
		printf (" ");
}

void print_bar (float value)
{
	int printed_char = 0;
	while ((value > 0.f) && (printed_char < 110)) {
		printed_char++;
		if (printed_char == 107) {
			printf ("▶");
		}
		else if (printed_char == 108) {
			printf (COLOR_INVERT);
			printf ("▶");
			printf (COLOR_UNINVERT);
		}
		else {
			print_char (value);
			value -= 1.f;
		}
	}

	while (printed_char < 110) {
		printed_char++;
		printf (" ");
	}
}

void print_histogram (std::vector < float >values)
{
	const int num_buckets = 50;
	float buckets[num_buckets];

	float min_value = FLT_MAX;
	float max_value = -FLT_MAX;
	float expectancy = 0.f;
	for (unsigned i = 0; i < values.size (); i++) {
		if (values[i] < min_value)
			min_value = values[i];
		if (values[i] > max_value)
			max_value = values[i];
		expectancy += values[i];
	}

	expectancy /= (float) values.size ();

	printf ("Value range %f - %f\n", min_value, max_value);

	min_value = 0.f;
	max_value = 3.f;

	for (int b = 0; b < num_buckets; b++)
		buckets[b] = 0.f;

	int break_even_bucket = (int) ((float) num_buckets * (1.0f - min_value) / (max_value - min_value));

	for (unsigned i = 0; i < values.size (); i++) {
		int bucket_index = (int) (num_buckets * (values[i] - min_value) / (max_value - min_value));
		// Fold the tail of the distribution.
		if (bucket_index >= num_buckets)
			bucket_index = num_buckets - 1;
		buckets[bucket_index] += 0.00012f;
	}

	float total = 0.00012f * values.size ();
	float counted = 0.f;
	for (int b = 0; b < num_buckets; b++) {
		float value = buckets[b];
		counted += value;
		if (b < break_even_bucket)
			printf (COLOR_RED);
		else if (b == break_even_bucket)
			printf (COLOR_YELLOW);
		else
			printf (COLOR_GREEN);

		print_bar (value);

		printf ("│ %.0f%% e=%f", 100.f * counted / total, (float) b / (float) num_buckets * (max_value - min_value) + min_value);
		printf ("\n");
	}
	printf (COLOR_NORMAL);
}


