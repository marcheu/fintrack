#ifndef _PORTFOLIO_H_
#define _PORTFOLIO_H_

#include "data_series.h"

class portfolio {
      public:
	// the ratio of each stock in the portfolio -- adds up to 1.0
	float proportions[1024];

	int size_;

	void read (const char *file_name, std::vector < data_series > data);
	void write (const char *file_name);

	void randomize (std::vector < data_series > data);
	void normalize ();
	void max_proportions (std::vector < data_series > historical_data);
	void print (std::vector < data_series > historical_data);
	void print_as_file (std::vector < data_series > historical_data);
};

#endif
