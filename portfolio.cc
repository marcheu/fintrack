#include "constants.h"
#include "portfolio.h"
#include "util.h"

void portfolio::read (const char *file_name, std::vector < data_series > data)
{
	char dummy_line[1024];

	FILE *f = fopen (file_name, "r");

	assert (f);

	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		char name[256];
		float value;
		unsigned i;
		int r = sscanf (dummy_line, "%s %f", name, &value);
		assert (r == 2);

		for (i = 0; i < data.size (); i++)
			if (!strcmp (data[i].name, name))
				break;

		if (i == data.size ()) {
			printf ("can't find %s\n", name);
			assert (0);
		}

		proportions[i] = value;
	}

	size_ = data.size ();

	fclose (f);
}

void portfolio::write (const char *file_name, std::vector < data_series > data)
{
	FILE *f = fopen (file_name, "wb");

	assert (f);

	for (int i = 0; i < size_; i++)
		if (proportions[i] >= PORTFOLIO_IGNORE_PROPORTION)
			fprintf (f, "%s %f\n", data[i].name, proportions[i]);
	fclose (f);
}

void portfolio::constant (std::vector < data_series > data)
{
	size_ = data.size ();

	for (int i = 0; i < size_; i++)
		proportions[i] = 1.0f;
}

void portfolio::randomize (std::vector < data_series > data)
{
	size_ = data.size ();

	for (int i = 0; i < size_; i++)
		proportions[i] = (float) (rand () % 100 + 1);
}

void portfolio::normalize ()
{
	float sum = 0.f;
	for (int i = 0; i < size_; i++) {
		if (proportions[i] < 0.f)
			proportions[i] = 0.f;
		sum += proportions[i];
	}

	for (int i = 0; i < size_; i++)
		proportions[i] /= sum;
}

void portfolio::max_proportions (std::vector < data_series > historical_data)
{
	normalize ();



	const float limit = PORTFOLIO_MAX_PROPORTION;

	float over = 0.f;

	for (int i = 0; i < size_; i++)
		if (proportions[i] > limit) {
			over += (proportions[i] - limit);
			proportions[i] = limit;
		}

	while (over > 0.0001f) {
		int rand_index = rand () % size_;
		if (proportions[rand_index] < limit) {
			float delta = limit - proportions[rand_index];
			proportions[rand_index] += delta;
			over -= delta;
		}
	}

	normalize ();
}

void portfolio::print (std::vector < data_series > historical_data)
{
	printf ("|");
	for (int i = 0; i < size_; i++)
		if (proportions[i] >= PORTFOLIO_IGNORE_PROPORTION)
			printf ("%-5s|", historical_data[i].name);
	printf ("\n|");
	for (int i = 0; i < size_; i++) {
		if (proportions[i] < PORTFOLIO_IGNORE_PROPORTION)
			continue;

		if (proportions[i] > 0.3)
			printf (COLOR_RED);
		else if (proportions[i] > 0.2)
			printf (COLOR_YELLOW);
		else if (proportions[i] > 0.1)
			printf (COLOR_GREEN);

		printf ("%.3f", proportions[i]);
		printf (COLOR_NORMAL);
		printf ("|");
	}
	printf ("\n");
}

void portfolio::print_as_file (std::vector < data_series > historical_data)
{
	for (int i = 0; i < size_; i++)
		if (proportions[i] >= PORTFOLIO_IGNORE_PROPORTION)
			printf ("%s %f\n", historical_data[i].name, proportions[i]);
}
