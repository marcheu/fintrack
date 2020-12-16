
#include "includes.h"

#include "data_series.h"
#include "loader.h"

const char *data_dir_etf = "financial_data/etf/";
const char *data_dir_stock = "financial_data/stock/";

class file_iterator {
      public:
	void reset (bool use_stocks) {
		etf_dir = false;
		stock_dir = !use_stocks;
	} bool run (char *filename) {
		static DIR *d = NULL;

		if (!etf_dir) {
			d = opendir (data_dir_etf);
			assert (d);
			etf_dir = true;
		}

	      again:
		struct dirent *dir = readdir (d);

		if (dir == NULL && !stock_dir) {
			closedir (d);
			d = opendir (data_dir_stock);
			assert (d);
			dir = readdir (d);
			stock_dir = true;
		}

		if (dir != NULL) {
			if (dir->d_type == DT_DIR)
				goto again;
			strcpy (filename, dir->d_name);
			return true;
		}
		else {
			closedir (d);
			return false;
		}
	}
      private:
	bool etf_dir;
	bool stock_dir;

};

int loader::number_of_csv (bool use_stocks)
{
	int count = 0;
	char dummy_line[1024];
	file_iterator it;

	it.reset (use_stocks);
	while (it.run (dummy_line))
		count++;

	return count;
}

void loader::load_csv (char *file_name, date start, date end, data_series & data)
{
	char file_path[256];
	char dummy_line[1024];
	strcpy (file_path, data_dir_etf);
	strcat (file_path, file_name);
	FILE *f = fopen (file_path, "rb");

	if (!f) {
		strcpy (file_path, data_dir_stock);
		strcat (file_path, file_name);
		f = fopen (file_path, "rb");
	}

	assert (f);

	// Read the header and throw it away
	assert (fgets (dummy_line, sizeof (dummy_line), f));

	int count = 0;
	date d;
	float open_price, high, low, close, adj_close;
	int volume;
	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		int r = sscanf (dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);
		assert (r == 3);

		if ((d >= start) && (d <= end))
			count++;
	}

	data.values = (float *) malloc (count * sizeof (float));


	rewind (f);

	// Read the header and throw it away
	assert (fgets (dummy_line, sizeof (dummy_line), f));

	int i = 0;
	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		int r = sscanf (dummy_line, "%d-%d-%d,%f,%f,%f,%f,%f,%d\n", &d.year, &d.month, &d.day, &low, &open_price, &close, &volume, &adj_close);
		if ((d >= start) && (d <= end)) {
			assert (r == 9);
			data.values[i] = close;
			i++;
		}

	}

	assert (i == count);

	data.size = count;

	fclose (f);
}

void loader::find_start_end_date (char *file_name, data_series & data)
{
	char file_path[256];
	char dummy_line[1024];
	strcpy (file_path, data_dir_etf);
	strcat (file_path, file_name);
	FILE *f = fopen (file_path, "rb");

	if (!f) {
		strcpy (file_path, data_dir_stock);
		strcat (file_path, file_name);
		f = fopen (file_path, "rb");
	}

	assert (f);

	// Read the header and throw it away
	assert (fgets (dummy_line, sizeof (dummy_line), f));

	date d;
	assert (fgets (dummy_line, sizeof (dummy_line), f));
	int r = sscanf (dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);
	assert (r == 3);
	date min_date = d;

	date max_date = d;

	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		int r = sscanf (dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);

		assert (r == 3);

		if (max_date < d)
			max_date = d;
		if (min_date > d)
			min_date = d;
	}

	fclose (f);

	data.start = min_date;
	data.end = max_date;
	strcpy (data.name, file_name);
	data.name[strlen (data.name) - 4] = 0;
}

void loader::load_all_series (std::vector < data_series > &data, bool use_stocks)
{
	int num_files = number_of_csv (use_stocks);
	int i;
	char dummy_line[1024];
	file_iterator it;

	assert (num_files >= 1);

	// Create all our series
	for (i = 0; i < num_files; i++) {
		data_series *d = (data_series *) malloc (sizeof (data_series));
		data.push_back (*d);
	}

	// For each series, find min/max dates
	i = 0;
	it.reset (use_stocks);
	while (it.run (dummy_line)) {
		find_start_end_date (dummy_line, data[i]);
		i++;
	}

	// Compute overlapping date range
	date min_date = data[0].start;
	date max_date = data[0].end;
	for (int i = 1; i < num_files; i++) {
		if (data[i].start > min_date)
			min_date = data[i].start;
		if (data[i].end < max_date)
			max_date = data[i].end;
	}

	printf ("Overall date range: ");
	min_date.print ();
	max_date.print ();
	printf ("\n");

	// Read all the data for the range we want
	i = 0;
	it.reset (use_stocks);
	while (it.run (dummy_line)) {
		printf ("[%3d] loading %5s ( ", i, dummy_line);
		data[i].start.print ();
		data[i].end.print ();
		printf (") ... ");
		load_csv (dummy_line, min_date, max_date, data[i]);
		printf ("got %d records\n", data[i].size);
		i++;
	}
}
