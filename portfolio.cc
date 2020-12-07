#include "portfolio.h"
#include "util.h"

void portfolio::read(const char* file_name, std::vector<data_series> data) {
	char dummy_line[1024];

	FILE* f = fopen(file_name, "rb");

	assert(f);

	while(fgets(dummy_line, sizeof(dummy_line), f)) {
		char name[256];
		float value;
		unsigned i;
		int r = sscanf(dummy_line, "%s %f", name, &value);
		assert(r == 2);

		for(i = 0; i < data.size(); i++)
			if (!strcmp(data[i].name, name))
				break;

		assert(i < data.size());

		proportions[i] = value;
	}

	size_ = data.size();

	fclose(f);
}

void portfolio::write(const char* file_name) {
	assert(0);
}

void portfolio::randomize(std::vector<data_series> data)
{
	size_ = data.size();

	for(int i = 0; i < size_; i++)
		proportions[i] = (float)(rand() % 100 + 1);
}

void portfolio::normalize()
{
	float sum = 0.f;
	for(int i = 0; i < size_; i++) {
		if (proportions[i] < 0.f)
			proportions[i] = 0.f;
		sum += proportions[i];
	}

	for(int i = 0; i < size_; i++)
		proportions[i] /= sum;
}

void portfolio::max_proportions()
{
	normalize();

	bool redo = false;
	for(int i = 0; i < size_; i++)
		if (proportions[i] > 0.25f) {
			proportions[i] = 0.25f;
			redo = true;
		}

	if (redo)
		normalize();
}

void portfolio::print(std::vector<data_series> historical_data)
{
	printf("|");
	for(int i = 0; i < size_; i++)
		if (proportions[i] >= 0.003)
			printf("%-5s|", historical_data[i].name);
	printf("\n|");
	for(int i = 0; i < size_; i++) {
		if (proportions[i] < 0.003)
			continue;

		if (proportions[i] > 0.3)
		printf(COLOR_RED);
		else if (proportions[i] > 0.2)
		printf(COLOR_YELLOW);
		else if (proportions[i] > 0.1)
		printf(COLOR_GREEN);

		printf("%.3f", proportions[i]);
		printf(COLOR_NORMAL);
		printf("|");
	}
	printf("\n");
}


