#include "includes.h"
#include "util.h"

static void print_char (float v)
{
	if (v >= 8.f / 8.f)
		printf ("█");
	else if (v >= 7.f / 8.f)
		printf ("▉");
	else if (v >= 6.f / 8.f)
		printf ("▊");
	else if (v >= 5.f / 8.f)
		printf ("▋");
	else if (v >= 4.f / 8.f)
		printf ("▌");
	else if (v >= 3.f / 8.f)
		printf ("▍");
	else if (v >= 2.f / 8.f)
		printf ("▎");
	else if (v >= 1.f / 8.f)
		printf ("▏");
	else
		printf (" ");
}

void print_bar (float value)
{
	int printed_char = 0;
	while ((value > 0.f) && (printed_char < 110)) {
		printed_char++;
		if (printed_char == 107) {
			printf ("▶");
		}
		else if (printed_char == 108) {
			printf (COLOR_INVERT);
			printf ("▶");
			printf (COLOR_UNINVERT);
		}
		else {
			print_char (value);
			value -= 1.f;
		}
	}

	while (printed_char < 110) {
		printed_char++;
		printf (" ");
	}
}
