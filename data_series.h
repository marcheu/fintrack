#ifndef _DATA_SERIES_H_
#define _DATA_SERIES_H_

#include "date.h"
#include "includes.h"

static char sector_name[][32] = {
	"Materials             ",
	"Consumer Cyclical     ",
	"Financial Services    ",
	"Real Estate           ",
	"Consumer Defensive    ",
	"Healthcare            ",
	"Utilities             ",
	"Communication Services",
	"Energy                ",
	"Industrials           ",
	"Technology            ",
	"Bonds                 ",
	"Currency              "
};

#define NUM_SECTORS ARRAY_SIZE(sector_name)

struct data_series {
	float *values;
	int size;
	date start;
	date end;
	char name[1024];
	float sector_proportions[NUM_SECTORS];
	float leverage;
};

#endif
