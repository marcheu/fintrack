
#include "data_series.h"
#include "includes.h"
#include "loader.h"

#ifdef WIN32
const char *data_dir_etf = "financial_data\\etf\\";
const char *data_dir_stock = "financial_data\\stock\\";
const char *data_dir_test = "financial_data\\test\\";
const char *data_file_sectors = "financial_data/sectors/sectors.csv";
#else
const char *data_dir_etf = "financial_data/etf/";
const char *data_dir_stock = "financial_data/stock/";
const char *data_dir_test = "financial_data/test/";
const char *data_file_sectors = "financial_data/sectors/sectors.csv";
#endif

class file_iterator {
      public:
	virtual void reset (bool use_stocks) = 0;
	virtual bool run (char *filename) = 0;
	  virtual ~ file_iterator () {
	};
};

#ifdef WIN32
class file_iterator_test:public file_iterator {
      public:
	void reset (bool use_stocks) {
		strcpy (szDir, data_dir_test);
		strcat (szDir, "*");
		memset (&ffd, 0, sizeof (ffd));
		hFind = INVALID_HANDLE_VALUE;
	}
	bool run (char *filename) {
	      again:
		if (hFind == INVALID_HANDLE_VALUE)
			hFind = FindFirstFile (szDir, &ffd);
		else {
			if (!FindNextFile (hFind, &ffd))
				return false;
		}

		if (hFind == INVALID_HANDLE_VALUE)
			return false;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			goto again;

		strcpy (filename, ffd.cFileName);
		return true;
	}
      protected:
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	char szDir[MAX_PATH];
};

class file_iterator_real:public file_iterator_test {
      public:
	void reset (bool use_stocks) {
		if (use_stocks)
			strcpy (szDir, data_dir_stock);
		else
			strcpy (szDir, data_dir_etf);
		strcat (szDir, "*");
		memset (&ffd, 0, sizeof (ffd));
		hFind = INVALID_HANDLE_VALUE;
	}
};

#else
class file_iterator_test:public file_iterator {
      public:
	void reset (bool use_stocks) {
		d = NULL;
	} bool run (char *filename) {

		if (!d) {
			d = opendir (data_dir_test);
			assert (d);
		}

	      again:
		struct dirent *dir = readdir (d);

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
	~file_iterator_test () {
		d = NULL;
	}
      private:
	DIR * d;
};

class file_iterator_real:public file_iterator {
      public:
	void reset (bool use_stocks) {
		etf_dir = false;
		stock_dir = !use_stocks;
		d = NULL;
	} bool run (char *filename) {
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
	~file_iterator_real () {
		d = NULL;
	}
      private:
	bool etf_dir;
	bool stock_dir;
	DIR *d;
};
#endif

int loader::number_of_csv (file_iterator * it, bool use_stocks)
{
	int count = 0;
	char dummy_line[1024];

	it->reset (use_stocks);
	while (it->run (dummy_line))
		count++;

	return count;
}

void loader::load_csv (char *file_name, date start, date end, data_series & data)
{
	char file_path[256];
	char dummy_line[1024];
	strcpy (file_path, data_dir_etf);
	strcat (file_path, file_name);
	FILE *f = fopen (file_path, "r");

	if (!f) {
		strcpy (file_path, data_dir_stock);
		strcat (file_path, file_name);
		f = fopen (file_path, "r");
	}

	if (!f) {
		strcpy (file_path, data_dir_test);
		strcat (file_path, file_name);
		f = fopen (file_path, "r");
	}

	assert (f);

	// Read the header and throw it away
	char *s = fgets (dummy_line, sizeof (dummy_line), f);
	assert (s != NULL);

	int count = 0;
	date d;
	float open_price, high, low, close_price, volume, adj_close;
	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		int r = sscanf (dummy_line, "%d-%d-%d", &d.year, &d.month, &d.day);
		assert (r == 3);

		if ((d >= start) && (d <= end))
			count++;
	}

	data.values = (float *) malloc (count * sizeof (float));
	data.dates = (date *) malloc (count * sizeof (date));

	rewind (f);

	// Read the header and throw it away
	s = fgets (dummy_line, sizeof (dummy_line), f);
	assert (s != NULL);

	int i = 0;
	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		int r = sscanf (dummy_line, "%d-%d-%d,%f,%f,%f,%f,%f,%f\n", &d.year, &d.month, &d.day, &high, &low, &open_price, &close_price, &volume, &adj_close);
		if ((d >= start) && (d <= end)) {
			assert (r == 9);
			data.values[i] = close_price;
			data.dates[i] = d;
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
	FILE *f = fopen (file_path, "r");

	if (!f) {
		strcpy (file_path, data_dir_stock);
		strcat (file_path, file_name);
		f = fopen (file_path, "r");
	}

	if (!f) {
		strcpy (file_path, data_dir_test);
		strcat (file_path, file_name);
		f = fopen (file_path, "r");
	}

	assert (f);

	// Read the header and throw it away
	char *s = fgets (dummy_line, sizeof (dummy_line), f);
	assert (s != NULL);

	date d;
	s = fgets (dummy_line, sizeof (dummy_line), f);
	assert (s != NULL);
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

static int index_of (std::vector < data_series > &data, const char *name)
{
	for (unsigned i = 0; i < data.size (); i++)
		if (!strcmp (data[i].name, name))
			return i;

	printf ("Couldn't find %s\n", name);
	return -1;
}

void loader::load_sectors (std::vector < data_series > &data)
{
	FILE *f = fopen (data_file_sectors, "r");
	assert (f);

	char dummy_line[1024];

	// Read the header and throw it away
	char *s = fgets (dummy_line, sizeof (dummy_line), f);
	assert (s != NULL);

	for (unsigned i = 0; i < data.size (); i++)
		data[i].leverage = 0.f;

	while (fgets (dummy_line, sizeof (dummy_line), f)) {
		float values[NUM_SECTORS], leverage;
		char *data_line = &dummy_line[0];
		int index;
		while (data_line[0] != ',')
			data_line++;
		data_line[0] = 0;
		data_line++;
		int r = sscanf (data_line, "%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f,	%f\n",
				&values[0],
				&values[1],
				&values[2],
				&values[3],
				&values[4],
				&values[5],
				&values[6],
				&values[7],
				&values[8],
				&values[9],
				&values[10],
				&values[11],
				&values[12],
				&leverage);
		assert (r == 14);
		index = index_of (data, dummy_line);
		if (index == -1)
			continue;
		memcpy (data[index].sector_proportions, values, sizeof (float) * NUM_SECTORS);
		data[index].leverage = leverage;
	}

	for (unsigned i = 0; i < data.size (); i++)
		if (data[i].leverage == 0.f)
			printf ("Warning: %s doesn't have sectors\n", data[i].name);
}

void loader::load_all_series (std::vector < data_series > &data, bool use_stocks, bool use_test)
{
	file_iterator *it;
	if (use_test)
		it = new file_iterator_test ();
	else
		it = new file_iterator_real ();

	it->reset (use_stocks);
	int num_files = number_of_csv (it, use_stocks);
	int i;
	char dummy_line[1024];

	assert (num_files >= 1);

	// Create all our series
	data.resize (num_files);

	// For each series, find min/max dates
	i = 0;
	it->reset (use_stocks);
	while (it->run (dummy_line)) {
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
	it->reset (use_stocks);
	while (it->run (dummy_line)) {
		printf ("[%3d] loading %5s ( ", i, dummy_line);
		data[i].start.print ();
		data[i].end.print ();
		printf (") ... ");
		load_csv (dummy_line, min_date, max_date, data[i]);
		printf ("got %d records\n", data[i].size);
		i++;
	}

	// read the sectors
	load_sectors (data);

	delete it;
}
