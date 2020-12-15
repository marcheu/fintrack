#ifndef _DATE_H_
#define _DATE_H_

#include "includes.h"

class date {
      public:
	int year;
	int month;
	int day;

	friend bool operator>= (const date & d1, const date & d2) {
		if (d1.year > d2.year)
			return true;
		else if (d1.year < d2.year)
			return false;

		if (d1.month > d2.month)
			return true;
		else if (d1.month < d2.month)
			return false;

		if (d1.day > d2.day)
			return true;
		else if (d1.day < d2.day)
			return false;
		return true;
	};

	friend bool operator<= (const date & d1, const date & d2) {
		if (d1.year < d2.year)
			return true;
		else if (d1.year > d2.year)
			return false;

		if (d1.month < d2.month)
			return true;
		else if (d1.month > d2.month)
			return false;

		if (d1.day < d2.day)
			return true;
		else if (d1.day > d2.day)
			return false;

		return true;
	}

	friend bool operator< (const date & d1, const date & d2) {
		return !(d1 >= d2);
	}

	friend bool operator> (const date & d1, const date & d2) {
		return !(d1 <= d2);
	}

	void print () {
		printf ("%4d-%02d-%02d ", year, month, day);
	}
};

#endif
