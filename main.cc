#include "stochastic_optimization.h"
#include "includes.h"
#include "loader.h"

int main(int argc, char* argv[])
{
	srand(time(NULL));

	loader l;

	std::vector<data_series> data;

	l.load_all_series(data);

	stochastic_optimization(data);

	return 0;
}

