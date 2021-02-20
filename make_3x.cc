#include "make_3x.h"

// Makes a 3x leveraged fund from a given index. This is useful to create history for 3x leveraged ETFs like WANT (from VCR)
void make_3x (std::vector < data_series > data, portfolio & p, char *filename)
{
	FILE *f = fopen (filename, "w");
	assert (f);

	fprintf (f, "Date,High,Low,Open,Close,Volume,Adj Close\n");

	double value;
	for (int day = 0; day < data[0].size - 1; day++) {
		value = 0.;
		for (unsigned stock = 0; stock < data.size (); stock++) {
			value += p.proportions[stock];
			// Each day, assume perfect 3x leverage, and skim 0.98 annualized fees
			// 1.0098 is the right value below, but 1.02 is a value that accounts better for slippage etc.
			p.proportions[stock] *= (1. + (data[stock].values[day + 1] / data[stock].values[day] - 1.) * 3.) / pow (1.02, 1. / 253.);
		}
		fprintf (f, "%04d-%02d-%02d,", data[0].dates[day].year, data[0].dates[day].month, data[0].dates[day].day);
		fprintf (f, "%f,%f,%f,%f,%f,%f\n", value, value, value, value, value, value);
	}
	fclose (f);

}
