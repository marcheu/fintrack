#ifndef _DATA_SERIES_H_
#define _DATA_SERIES_H_

#include "date.h"
#include "includes.h"

struct data_series {
	float *values;
	int size;
	date start;
	date end;
	char name[1024];
};

#endif
