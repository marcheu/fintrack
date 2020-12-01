
#include "includes.h"

#include "data_series.h"
#include "loader.h"

const char* data_dir = "financial_data/";

int loader::number_of_csv()
{
	DIR *d;
	struct dirent *dir;
	int count = 0;
	d = opendir(data_dir);
	assert(d);

	while ((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_DIR)
			continue;
		count++;
	}

	closedir(d);
	return count;
}

void loader::load_csv(char* file_name, date start, date end, data_series &data)
{
	char file_path[256];
	char dummy_line[1024];
	strcpy(file_path, data_dir);
	strcat(file_path, file_name);
	FILE* f = fopen(file_path, "rb");

	assert(f);

	// read the header and throw it away
	assert(fgets(dummy_line, sizeof(dummy_line), f));

	int count = 0;
	date d;
	float open_price, high, low, close, adj_close;
	int volume;
	while(fgets(dummy_line, sizeof(dummy_line), f)) {
		int r = sscanf(dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);
		assert(r == 3);

		if ((d >= start) && (d <= end))
			count++;
	}

	data.values = (float*) malloc(count * sizeof(float));


	rewind(f);

	// read the header and throw it away
	assert(fgets(dummy_line, sizeof(dummy_line), f));

	int i = 0;
	while(fgets(dummy_line, sizeof(dummy_line), f)) {
		int r = sscanf(dummy_line, "%d-%d-%d,%f,%f,%f,%f,%f,%d\n", &d.year, &d.month, &d.day, &open_price, &high, &low, &close, &adj_close, &volume);
		if ((d >= start) && (d <= end))
		{
			assert(r == 9);
			data.values[i] = close;
			i++;
		}
			
	}

	assert(i == count);

	data.size = count;

	fclose(f);
}

void loader::find_start_end_date(char* file_name, data_series &data)
{
	char file_path[256];
	char dummy_line[1024];
	strcpy(file_path, data_dir);
	strcat(file_path, file_name);
	FILE* f = fopen(file_path, "rb");

	assert(f);

	// read the header and throw it away
	assert(fgets(dummy_line, sizeof(dummy_line), f));

	date d;
	assert(fgets(dummy_line, sizeof(dummy_line), f));
	int r = sscanf(dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);
	assert(r == 3);
	date min_date = d;
	date max_date = d;

	while(fgets(dummy_line, sizeof(dummy_line), f)) {
		int r = sscanf(dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);

		assert(r == 3);

		if (max_date < d)
			max_date = d;
		if (min_date > d)
			min_date = d;
	}

	fclose(f);

	data.start = min_date;
	data.end = max_date;
	strcpy(data.name, file_name);
}

void loader::load_all_series(std::vector<data_series> &data)
{
	int num_files = number_of_csv();
	int i;
	DIR *d;
	struct dirent *dir;

	assert(num_files >= 1);

	// Create all our series
	for(i = 0; i < num_files; i++) {
		data_series* d = (data_series*)malloc(sizeof(data_series));
		data.push_back(*d);
	}

	// For each series, find min/max dates
	d = opendir(data_dir);
	assert(d);

	i = 0;
	while ((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_DIR)
			continue;
		find_start_end_date(dir->d_name, data[i]);
		printf("min/max %s ", dir->d_name);
		data[i].start.print();
		data[i].end.print();
		printf("\n");
		i++;
	}
	closedir(d);

	// Compute overlapping date range
	date min_date = data[0].start;
	date max_date = data[0].end;
	for(int i = 1; i < num_files; i++)
	{
		if (data[i].start > min_date)
			min_date = data[i].start;
		if (data[i].end < max_date)
			max_date = data[i].end;
	}

	printf("Overall date range: ");
	min_date.print();
	max_date.print();
	printf("\n");

	// Read all the data for the range we want
	d = opendir(data_dir);
	assert(d);

	i = 0;
	while ((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_DIR)
			continue;
		printf("[%d] loading %s ... ", i, dir->d_name);
		load_csv(dir->d_name, min_date, max_date, data[i]);
		printf("got %d records\n", data[i].size);
		i++;
	}
	closedir(d);
}
