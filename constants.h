#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Number of trading days in each year
// This depends on the year, as such might create a slight bias
#define TRADING_DAYS_PER_YEAR 253

// The length of a single simulation in number of days. This is typically
// the interval between two portfolio rebalances, or the maximum time you
// can stomach a drawdown
#define MONTE_CARLO_SIMULATION_DURATION (TRADING_DAYS_PER_YEAR / 2)

// Max number of simultaneous tickers for the GPU simulation
#define GPU_SIMULATION_MAX_STOCKS 512

// The maximum proportion of the portfolio allocated to a single investment
#define PORTFOLIO_MAX_PROPORTION 0.4f

// Below this proportion, we ignore an entry in the portfolio.
// This is 0.1%
#define PORTFOLIO_IGNORE_PROPORTION 0.001f

#endif
