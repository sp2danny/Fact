
#pragma once

#include <complex>
#include <vector>

#include "graph.h"

typedef long double Flt;

typedef unsigned long UL;

typedef std::complex<Flt> Cmplx;

Cmplx step(Cmplx c, Cmplx z);

struct Point
{
	enum { in, calc, out } status = calc;
	unsigned long iter = 0;
	Cmplx z = {0,0};
};

struct Map
{
	UL width, height;
	Flt scale_x, scale_y;
	Flt center_x, center_y;
	bool map_all_done = false;
	std::vector<Point> points;
	std::size_t to_index(UL x, UL y) const;
	Flt to_xpos(UL x) const;
	Flt to_ypos(UL y) const;
	void generate_init();
	enum Status { all_done, was_updated, no_change };
	Status generate(UL cap);
	Image makeimage(float mod, UL fuc = 0);
};

template<typename T>
T clamp(T val, T min, T max)
{
	if (val<min) val=min;
	if (val>max) val=max;
	return val;
}

RGB col(Point p, float mod);

void movie_maker(Map& m, UL cap);

