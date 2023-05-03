#include "features.h"


int math::sin(int a, int b, int c) {
	// motst�ende sidan / hypotenusan
	return (c / a);
}

int math::cos(int a, int b, int c) {
	// n�rliggande sidan / hypotenusan
	return (b / a); 
}

int math::tan(int a, int b, int c) {
	// motst�ende sidan / n�rst�ende sidan
	return (c / b);
}


