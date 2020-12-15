#ifndef _LOADER_H_
#define _LOADER_H_

#include "data_series.h"
#include "date.h"

class loader {
      public:
	int number_of_csv (bool use_stocks);
	void load_all_series (std::vector < data_series > &data, bool use_stocks);
      private:
	void load_csv (char *file_name, date start, date end, data_series & data);
	void find_start_end_date (char *filename, data_series & data);
};

#endif
