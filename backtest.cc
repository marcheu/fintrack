#include "backtest.h"
#include "constants.h"



static void write_portfolio (std::vector < data_series > &data, portfolio & p, const char *filename)
{
	FILE *f_portfolio = fopen (filename, "w");
	assert (f_portfolio);

	double value;
	for (int day = 0; day < data[0].size - 1; day++) {
		value = 0.;
		for (unsigned stock = 0; stock < data.size (); stock++) {
			p.proportions[stock] *= data[stock].values[day + 1] / data[stock].values[day];
			value += p.proportions[stock];
		}
		fprintf (f_portfolio, "%d %f\n", day, value);
	}
	fclose (f_portfolio);
}

static char *month_name (int month)
{
	assert (month >= 1);
	assert (month <= 12);
	static char n[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	return n[month - 1];
}

void backtest_portfolio (std::vector < data_series > &data, portfolio & p)
{
	int r;

	// Create temporary dir
	r = system ("mkdir -p " TMP_DIR);

	write_portfolio (data, p, TMP_DIR "tmp_data.txt");

	portfolio my_portfolio;
	my_portfolio.read ("portfolios/my_portfolio.txt", data);
	my_portfolio.normalize ();
	write_portfolio (data, my_portfolio, TMP_DIR "tmp_my_data.txt");

	portfolio hedgfundie;
	hedgfundie.read ("portfolios/hedgfundie.txt", data);
	hedgfundie.normalize ();
	write_portfolio (data, hedgfundie, TMP_DIR "tmp_hedg_data.txt");

	portfolio tqqq;
	tqqq.read ("portfolios/TQQQ.txt", data);
	tqqq.normalize ();
	write_portfolio (data, tqqq, TMP_DIR "tmp_tqqq_data.txt");

	portfolio upro;
	upro.read ("portfolios/UPRO.txt", data);
	upro.normalize ();
	write_portfolio (data, upro, TMP_DIR "tmp_upro_data.txt");

	portfolio vgt;
	vgt.read ("portfolios/VGT.txt", data);
	vgt.normalize ();
	write_portfolio (data, vgt, TMP_DIR "tmp_vgt_data.txt");

	portfolio vti_portfolio;
	for (unsigned stock = 0; stock < data.size (); stock++)
		if (!strcmp (data[stock].name, "VTI"))
			vti_portfolio.proportions[stock] = 1.0f;
		else
			vti_portfolio.proportions[stock] = 0.0f;
	write_portfolio (data, vti_portfolio, TMP_DIR "tmp_vti_data.txt");

	char *cmd = (char *) calloc (1, 16384 * sizeof (char));
	strcpy (cmd, "gnuplot -background white <<- EOF\nset term x11 persist\nset xtics (");

	int current_year = data[0].dates[0].year;
	int current_month = data[0].dates[0].month;
	for (int day = 0; day < data[0].size - 1; day++) {
		if (data[0].dates[day].year > current_year) {
			current_year = data[0].dates[day].year;
			current_month = data[0].dates[day].month;

			char tmp[128];
			sprintf (tmp, "\"%d\" %d,", current_year, day);
			strcat (cmd, tmp);
		}
		else if (data[0].dates[day].month > current_month) {
			current_month = data[0].dates[day].month;

			char tmp[128];
			sprintf (tmp, "\"%s\" %d,", month_name (current_month), day);
			strcat (cmd, tmp);
		}
	}
	strcat (cmd, "\" \" 9999)");
	strcat (cmd, "\nset xtics rotate by 90 right\nset grid\nset tics scale 0\nset style line 12 lc rgb '#D8D8D8' lt 1 lw 1\nset grid back ls 12\nplot \"" TMP_DIR "tmp_data.txt\" w l, \"" TMP_DIR "tmp_vti_data.txt\" w l, \"" TMP_DIR "tmp_my_data.txt\" w l, \"" TMP_DIR "tmp_hedg_data.txt\" w l , \"" TMP_DIR "tmp_tqqq_data.txt\" w l, \"" TMP_DIR "tmp_upro_data.txt\" w l, \"" TMP_DIR "tmp_vgt_data.txt\" w l \nEOF");

	assert (strlen (cmd) < 16000);

	r = system (cmd);
	assert (!r);
	free (cmd);
}
