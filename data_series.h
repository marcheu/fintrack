#include "includes.h"
#ifndef _DATA_SERIES_H_
#define _DATA_SERIES_H_

#include "date.h"

struct data_series {
	float *values;
	int size;
	date start;
	date end;
};

#endif
