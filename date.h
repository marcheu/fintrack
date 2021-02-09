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

	int day_of_the_week () {
		int adjustment, mm, yy;

		adjustment = (14 - month) / 12;
		mm = month + 12 * adjustment - 2;
		yy = year - adjustment;
		return (day + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400 - 1) % 7;
	}

	void print () {
		printf ("%4d-%02d-%02d ", year, month, day);
	}
};

#endif
