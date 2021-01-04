#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Number of trading days in each year
// This depends on the year, as such might create a slight bias
#define TRADING_DAYS_PER_YEAR 253

// The length of a block of trading days for the simulation
// If it's too small you miss long up/down trends and get an erroneous stddev
// If it's too big the results are just a replay of history
#define MONTE_CARLO_BLOCK_SIZE 20

#endif
