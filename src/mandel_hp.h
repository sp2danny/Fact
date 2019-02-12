
#pragma once

#include <cmath>
#include <complex>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <mutex>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "graph.h"

#include "gmpxx.h"

/*
struct Flt : mpf_class
{
	static constexpr int N = 512;
	Flt() : mpf_class(0.0, N) { }
	explicit Flt(double d) : mpf_class(d, N) { }
	explicit Flt(std::string s) : mpf_class(s.c_str(), N) {}
	template<typename T>
	Flt(T&& arg)
		: Flt()
	{
		((mpf_class&)*this) = std::forward<T>(arg);
	}
	template<typename T>
	Flt& operator=(T&& arg)
	{
		((mpf_class&)*this) = std::forward<T>(arg);
		return *this;
	}
	explicit operator double() { return get_d(); }
};
inline Flt from_string(const std::string& str)
{
	return Flt{str};
}
*/

/* */
typedef double Flt;

inline Flt from_string(const std::string& str)
{
	std::stringstream ss;
	ss.str(str);
	double d;
	ss >> d;
	return d;
}
/* */

inline bool isnan(const Flt&) { return false; }
inline bool isinf(const Flt&) { return false; }
Flt copysign(const Flt& a,const Flt& b);

typedef std::uint32_t UL;

typedef std::complex<Flt> Cmplx;

Cmplx step(Cmplx c, Cmplx z);

constexpr UC pt_normal = 1;
constexpr UC pt_ep_hor = 2;
constexpr UC pt_ep_ver = 3;
constexpr UC pt_black  = 4;

struct Point
{
	enum { in, calc, out } status = calc;
	UL iter = 0;
	float over = 0.0f;
	Cmplx z = {0.0,0.0};
	bool docalc(const Cmplx& c, UL cap);
	void init(const Cmplx& c);
	UC pixtype = 0;
};

typedef float (*ModFunc)(double);

typedef std::vector<Point> Scanline;

struct Map;

struct LineCache
{
	UL cap;
	UL eff_cap;
	UL skip_count;
	UL inskip;
	bool display;
	UL y_start, y_count;
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
	Status generate(UL cap, bool display=false, bool extrap=false);
	Image makeimage(float mod, UL upc=0);

	UL generate_10(UL cap, bool display=false);

	UL generate_N_threaded(int n, UL cap, bool display=false);
	Image makeimage_N(int n, ModFunc);

	void saveblob(int N, std::ostream&);
	
private:
	void generate_N_init(int n, bool disp);
	void generate_init_rest();
	RGB extrapolate(float x, float y, float mod);
	std::vector<Flt> vfx, vfy;

};

template<typename T>
T clamp(T val, T min, T max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}

RGB col(const Point& p, float mod);

struct Updater
{
	static void Init(UL);
	static void Tick();
	static void Display();
	static int Get();
private:
	static std::mutex lck;
	static UL count, max, last;
};

