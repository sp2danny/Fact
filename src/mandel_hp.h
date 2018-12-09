
#pragma once

#include <complex>
#include <vector>

#include "graph.h"

#include "gmpxx.h"


struct Flt : mpf_class
{
	static constexpr int N = 256;
	Flt() : mpf_class(0.0, N) { }
	Flt(double d) : mpf_class(d, N) { }
	Flt(std::string s) : mpf_class(s.c_str(), N) {}
	template<typename T>
	Flt(T&& arg)
		: Flt()
	{
		((mpf_class&)*this) = std::forward<T>(arg);
	}
};

typedef unsigned long UL;

typedef std::complex<Flt> Cmplx;

Cmplx step(Cmplx c, Cmplx z);

struct Point
{
	enum { in, calc, out } status = calc;
	unsigned long iter = 0;
	float over;
	Cmplx z = {0.0,0.0};
	bool docalc(const Cmplx& c, UL cap);
	void init(const Cmplx& c);
};

typedef float (*ModFunc)(double);

typedef std::vector<Point> Scanline;

struct Map;

struct LineCache
{
	UL cap;
	UL eff_cap;
	UL skip_count;
	bool display;
	std::vector<UL> scanlines;
	Map& map;
	std::vector<Flt>& vfx;
	std::vector<Flt>& vfy;
};

void execute(LineCache*);

struct Map
{
	UL width, height;
	Flt scale_x, scale_y;
	Flt center_x, center_y;
	Flt zoom_mul;
	bool map_all_done = false;
	std::vector<Scanline> points;
	UL new_w, new_h;
	Point& get(UL x, UL y);
	Flt to_xpos(UL x) const;
	Flt to_ypos(UL y) const;
	void generate_init();
	void generate_odd(UL cap);
	enum Status { all_done, was_updated, no_change };
	Status generate(UL cap, bool display=false);
	Image makeimage(float mod, UL upc=0);
	UL generate_10(UL cap, bool display=false);
	UL generate_10_threaded(UL cap, bool display=false);
	Image makeimage_N(int n, ModFunc);
	RGB extrapolate(float x, float y, float mod);
};

template<typename T>
T clamp(T val, T min, T max)
{
	if (val<min) val=min;
	if (val>max) val=max;
	return val;
}

RGB col(const Point& p, float mod);


