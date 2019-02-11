
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

/* */
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

/* */

/*
typedef double Flt;

inline Flt from_string(const std::string& str)
{
	std::stringstream ss;
	ss.str(str);
	double d;
	ss >> d;
	return d;
}
*/

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
	UL generate_10_threaded(UL cap, bool display=false);
	void generate_10_init(UL cap);
	Image makeimage_10_N(int n, ModFunc);
	RGB extrapolate(float x, float y, float mod);
	std::vector<Flt> vfx, vfy;

	void generate_25_init(bool disp);
	UL generate_25_threaded(UL cap, bool display=false);
	Image makeimage_25_N(int n, ModFunc);

	void generate_50_init(bool disp);
	UL generate_50_threaded(UL cap, bool display=false);
	Image makeimage_50_N(int n, ModFunc);
	
	template<int N>
	void generate_init(bool disp);
	template<int N>
	UL generate_threaded(UL cap, bool display=false);
	template<int N>
	Image makeimage_N(int n, ModFunc);

	void saveblob(int N, std::ostream&);

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

template<int N>
void Map::generate_init(bool disp)
{
	double zm = (double)zoom_mul;
	double tm = pow(zm, -N);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);
	
	if (disp)
	{
		std::cout << "Old size " << width << "x" << height << std::endl;
		std::cout << "New size " << new_w << "x" << new_h << std::endl;
	}

	points.resize(new_h);
	UL x,y;
	for (y=0; y<new_h; ++y)
		points[y].resize(new_w+3);

	Flt x_start = to_xpos(0);
	Flt x_stop  = to_xpos(width-1);
	Flt y_start = to_ypos(0);
	Flt y_stop  = to_ypos(height-1);
	Flt x_step  = (x_stop-x_start) / (new_w-1);
	Flt y_step  = (y_stop-y_start) / (new_h-1);

	vfx.clear(); vfy.clear();
	for (x=0; x<new_w; ++x)
		vfx.push_back(x_start + x_step*x);
	for (y=0; y<new_h; ++y)
		vfy.push_back(y_start + y_step*y);
}

template<int N>
UL Map::generate_threaded(UL cap, bool display)
{
	generate_init<N>(false);

	Updater::Init(new_h*3+4);
	if (display) Updater::Display();

	LineCache lc[4] = {
		{ cap, 0, 0, 0, display, 0, 0, *this, vfx, vfy },
		{ cap, 0, 0, 0,   false, 0, 0, *this, vfx, vfy },
		{ cap, 0, 0, 0,   false, 0, 0, *this, vfx, vfy },
		{ cap, 0, 0, 0,   false, 0, 0, *this, vfx, vfy },
	};

	UL i = 0;
	UL num = new_h / 4;
	UL ovr = new_h - (num*4);
	UL y = 0;
	lc[0].y_start = y; y += (lc[0].y_count = num + ovr);
	lc[1].y_start = y; y += (lc[1].y_count = num);
	lc[2].y_start = y; y += (lc[2].y_count = num);
	lc[3].y_start = y; y += (lc[3].y_count = num);

	UL maxout = 0;
	boost::thread tt[4];
	for (i=1; i<4; ++i)
	{
		tt[i] = boost::thread{&execute, lc+i};
	}
	execute(lc);
	boost::chrono::nanoseconds ns{250'000};
	int joined = 1;
	while (true)
	{
		if (display) Updater::Display();

		bool j = tt[joined].try_join_for(ns);
		if (j)
		{
			++joined;
			Updater::Tick();
			if (joined >= 4) break;
		}
	}
	UL sk = 0, is = 0;
	for (i=0; i<4; ++i)
	{
		if (lc[i].eff_cap > maxout)
			maxout = lc[i].eff_cap;
		sk += lc[i].skip_count;
		is += lc[i].inskip;
	}

	Updater::Tick();
	if (display) Updater::Display();

	if (display)
		std::cout << "skipped        : " << sk << " pixels, of wich " << is << " was inside \n";

	return maxout;
}

template<int N>
Image Map::makeimage_N(int n, ModFunc mf)
{
	Image img(width, height);
	double sx{scale_x};

	++n;
	double t0{zoom_mul};
	double tm  = powf(t0, -1);
	double tn  = powf(t0, -n);
	double tN  = powf(t0, -(N+1));
	double dif = tN-tm;
	double per = (tn-tm)/dif;

	double mod = mf(sx/tn);

	double xstart = (new_w - width)/2;
	xstart *= per;
	double ystart = (new_h - height)/2;
	ystart *= per;
	double xstep = double(new_w) / double(width);
	xstep = 1.0f + (xstep-1.0f)*(1.0f-per);
	double ystep = double(new_h) / double(height);
	ystep = 1.0f + (ystep-1.0f)*(1.0f-per);

	UL x,y;
	for (y=0; y<height; ++y)
	{
		double yf = ystart + y * ystep;
		for (x=0; x<width; ++x)
		{
			double xf = xstart + x * xstep;
			img.PutPixel(x,y,extrapolate(xf,yf, mod));
		}
	}

	return img;
}

