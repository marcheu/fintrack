#include "includes.h"
#include "loader.h"

int main(int argc, char* argv[])
{
	loader l;

	std::vector<data_series> data;

	l.load_all_series(data);

	return 0;
}

