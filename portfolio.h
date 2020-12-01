#ifndef _PORTFOLIO_H_
#define _PORTFOLIO_H_

struct portfolio {
	// the ratio of each stock in the portfolio -- adds up to 1.0
	float proportions[1024];
};

#endif
