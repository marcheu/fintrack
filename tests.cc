#include "loader.h"
#include "monte_carlo.h"
#include "portfolio.h"
#include "tests.h"
#include "util.h"

static void run_cpu_gpu_tests (std::vector < data_series > data)
{
	printf ("=== CPU/GPU correctness tests ===\n");

	for (int i = 0; i < 10; i++) {
		portfolio p;
		p.randomize (data);
		p.normalize ();

		int num_rounds = 1 << 16;
		float cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation;
		float gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation;

		// Run the same tests on the CPU and on the GPU and compare the results
		monte_carlo m_cpu (data, false);
		m_cpu.run (p, cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation, num_rounds);
		printf ("CPU monte carlo: e = %f σ = %f σd = %f\n", cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation);

		monte_carlo m_gpu (data, true);
		m_gpu.run (p, gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation, num_rounds);
		printf ("GPU monte carlo: e = %f σ = %f σd = %f\n", gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation);

		assert (fabs (cpu_expectancy - gpu_expectancy) < 0.00001f);
		assert (fabs (cpu_standard_deviation - gpu_standard_deviation) < 0.00001f);
		assert (fabs (cpu_downside_deviation - gpu_downside_deviation) < 0.00001f);
	}
}

static void run_performance_tests (std::vector < data_series > data)
{
	printf ("=== Performance tests ===\n");

	portfolio p;
	p.randomize (data);
	p.normalize ();

	int num_rounds = 1 << 16;
	float cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation;
	float gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation;

	// Run the same tests on the CPU and on the GPU and compare the results
	uint64_t before, after;

	monte_carlo m_cpu (data, false);

	before = get_time_us ();
	for (int i = 0; i < 10; i++)
		m_cpu.run (p, cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation, num_rounds);
	after = get_time_us ();

	printf ("CPU monte carlo: %f simulations per second\n", 10.f * num_rounds / ((after - before) / 1000000.0f));

	monte_carlo m_gpu (data, true);

	before = get_time_us ();
	for (int i = 0; i < 10; i++)
		m_gpu.run (p, gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation, num_rounds);
	after = get_time_us ();

	printf ("GPU monte carlo: %f simulations per second\n", 10.f * num_rounds / ((after - before) / 1000000.0f));
}

static void run_expectancy_tests(std::vector < data_series > data)
{
	printf ("=== Expectancy tests ===\n");
	portfolio p;

	p.proportions[0] = 1.0;
	for(int i = 1; i < p.size_; i++)
		p.proportions[i] = 0;

	int num_rounds = 1 << 16;
	float cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation;
	float gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation;

	monte_carlo m_cpu (data, false);
	m_cpu.run (p, cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation, num_rounds);
	printf ("CPU monte carlo: e = %f σ = %f σd = %f\n", cpu_expectancy, cpu_standard_deviation, cpu_downside_deviation);
	assert(fabs (cpu_expectancy - 1.1f) < 0.001f);

	monte_carlo m_gpu (data, true);
	m_gpu.run (p, gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation, num_rounds);
	printf ("GPU monte carlo: e = %f σ = %f σd = %f\n", gpu_expectancy, gpu_standard_deviation, gpu_downside_deviation);
	assert(fabs (gpu_expectancy - 1.1f) < 0.001f);
}

void run_tests ()
{
	std::vector < data_series > data;

	loader l;
	l.load_all_series (data, false, true);

	run_cpu_gpu_tests (data);
	run_expectancy_tests (data);
	run_performance_tests (data);

	printf ("Tests PASSED\n");
}
