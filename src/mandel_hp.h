
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
#include <functional>
#include <utility>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "graph.h"
#include "multilogger.hpp"

#include "gmpxx.h"

struct FltH : mpf_class
{
	static constexpr int N = 512;
	FltH() : mpf_class(0.0, N) { }
	explicit FltH(double d) : mpf_class(d, N) { }
	explicit FltH(std::string s) : mpf_class(s.c_str(), N) {}
	template<typename T>
	FltH(T&& arg)
		: FltH()
	{
		((mpf_class&)*this) = std::forward<T>(arg);
	}
	template<typename T>
	FltH& operator=(T&& arg)
	{
		((mpf_class&)*this) = std::forward<T>(arg);
		return *this;
	}
	explicit operator double() { return get_d(); }
};
inline FltH from_stringH(const std::string& str)
{
	return FltH{str};
}

typedef double FltL;
inline auto from_stringL = [](const std::string& str) { return std::stod(str); };

inline bool isnan(const FltH&) { return false; }
inline bool isinf(const FltH&) { return false; }
FltH copysign(const FltH& a,const FltH& b);

typedef std::uint32_t UL;

template<typename Flt>
std::complex<Flt> step(const std::complex<Flt>& c, const std::complex<Flt>& z);

constexpr UC pt_normal = 1;
constexpr UC pt_ep_hor = 2;
constexpr UC pt_ep_ver = 3;
constexpr UC pt_black  = 4;
constexpr UC pt_ep_msk = 2;

template<typename Flt>
struct Point
{
	enum { in, calc, out } status = calc;
	UL iter = 0;
	float over = 0.0;
	UC pixtype = 0;
	std::complex<Flt> z = {0.0,0.0};
	#ifndef NDEBUG
	std::complex<Flt> orig = {0.0,0.0};
	#endif
	bool docalc(const std::complex<Flt>& c, UL cap);
	void init(const std::complex<Flt>& c);
	RGB col(float mod) const;
	static Flt stepsize;
};

template<typename Flt> 
Flt Point<Flt>::stepsize{};

typedef float (*ModFunc)(float);

template<typename Flt>
using Scanline = std::vector<Point<Flt>>;

template<typename>
struct Map;

template<typename Flt>
struct LineCache
{
	LineCache(UL cap, bool display, Map<Flt>& map, bool first = false);

	UL cap;
	UL eff_cap;
	UL skip_count;
	UL inskip;
	bool display;
	bool first;
	UL y_start, y_count;
	Map<Flt>& map;
	std::vector<Flt>& vfx;
	std::vector<Flt>& vfy;

	static void execute(LineCache*);
	static void execute_dbl(LineCache*);

private:
	UL n, w;

	void base_init();
	void init_zero();
	void init_lim();
	void dc(Point<Flt>& p, const std::complex<Flt>& c);
	void even(), odd();
	bool ep_xy(UL, UL);
	bool ep_x(UL, UL);
	bool ep_y(UL, UL);
	bool setin();
	void all();

	UL xlo, xhi, ylo, yhi;
};

typedef std::ostream* OSP;

template<typename Flt>
struct Map
{
	UL width, height;
	Flt scale_x, scale_y;
	Flt center_x, center_y;
	Flt zoom_mul;
	bool map_all_done = false;
	std::vector<Scanline<Flt>> points;
	UL new_w, new_h;
	Point<Flt>& get(UL x, UL y);
	Flt to_xpos(UL x) const;
	Flt to_ypos(UL y) const;
	enum Status { all_done, was_updated, no_change };
	
	//Flt get_col_mod() const;
	void setZ(Flt z);
	
	// Singles
	void generate_init();
	Status generate(UL cap, bool display=false, bool extrap=false);
	Image makeimage(float mod, UL upc=0);

	// Batch
	UL generate_N_threaded(int n, UL cap, bool display=false);
	Image makeimage_N(int, ModFunc, OSP = nullptr);

	int count_dlb;
	Image dbl_makefull(UL);
	void setup_dbl(Flt);
	int generate_dbl(UL, bool, MultiLogger&);
	int shuffle_dbl();
	int sh_new_xcoord(int);
	int sh_new_ycoord(int);
	
	void new_out(std::string);

friend
	struct LineCache<Flt>;
private:
	void generate_N_init(int n, bool disp);
	void generate_init_rest();
	RGB extrapolate(float x, float y, float mod);
	std::vector<Flt> vfx, vfy;
	Image prep;
};

template<typename T>
inline T clamp(T val, T min, T max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}

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

