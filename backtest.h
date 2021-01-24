#ifndef _BACKTEST_H_
#define _BACKTEST_H_

#include "data_series.h"
#include "portfolio.h"

void backtest_portfolio (std::vector < data_series > &data, portfolio & p);

#endif
