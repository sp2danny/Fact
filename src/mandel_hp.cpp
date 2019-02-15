
#include "mandel_hp.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <mutex>
#include <climits>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

using namespace std::literals;

// ----------------------------------------------------------------------------

FltH copysign(const FltH& a,const FltH& b)
{
	bool aneg = a<0;
	bool bneg = b<0;
	if (aneg==bneg)
		return a;
	else
		return -a;
}

template<typename Flt>
std::complex<Flt> step(const std::complex<Flt>& c, const std::complex<Flt>& z)
{
	return z*z + c;
}

template std::complex<FltH> step<FltH>(const std::complex<FltH>&, const std::complex<FltH>&);
template std::complex<FltL> step<FltL>(const std::complex<FltL>&, const std::complex<FltL>&);

// ----------------------------------------------------------------------------

// *************
// *** Point ***
// *************

template<typename Flt>
bool Point<Flt>::docalc(const std::complex<Flt>& c, UL cap)
{
	UL& n = iter;
	if (n <= 1)
	{
		// first bulb
		double xld = (double)z.real();
		double yld = (double)z.imag();
		double y2 = yld * yld;
		double xldp1 = xld+1.0;
		if (((xldp1*xldp1) + y2) < 0.0625)
		{
			status = Point::in;
			pixtype = pt_black;
			return true;
		}
		// main cardoid
		double xx = xld - 0.25;
		xx *= xx;
		xx += y2;
		double pp = sqrt(xx);
		if (xld < (pp - 2.0*(pp*pp) + 0.25))
		{
			status = Point::in;
			pixtype = pt_black;
			return true;
		}
	}

	bool did_smth = false;

	while (true)
	{
		if (n >= cap)
		{
			pixtype = pt_black;
			break;
		}

		auto zre = z.real();
		Flt  re_sq = zre * zre;
		auto zim = z.imag();
		Flt  im_sq = zim * zim;
		auto az2 = (double)re_sq + (double)im_sq;
		if (az2 > 4.0)
		{
			did_smth = true;
			status = Point::out;
			over = sqrtf(az2);
			pixtype = pt_normal;
			break;
		}

		z.real(re_sq-im_sq);
		auto ab = zre * zim;
		z.imag(ab+ab);
		z += c;
		++n;
	}

	return did_smth;
}

template<typename Flt>
void Point<Flt>::init(const std::complex<Flt>& c)
{
	status = Point::calc;
	iter = 1;
	z = c;
}

template<typename Flt>
RGB Point<Flt>::col(float mod) const
{
	const Point& p = *this;
	if (p.status != Point::out)
	{
		return RGB{0,0,0};
	}
	auto x = p.iter;

	static const float pi2 = 3.1415926536f * 2;
	static const float ilg2 = 1.0f / log(2.0f);
	float f = fmod((float)x, mod) + 1.0f;
	f /= mod;

	f -= (float)log(log(p.over) * ilg2) * ilg2 / mod;
	f *= pi2;
	float r = 0.5f + 0.5f*std::sin(f + pi2 * 0.00000f);
	float g = 0.5f + 0.5f*std::sin(f + pi2 * 0.33333f);
	float b = 0.5f + 0.5f*std::sin(f + pi2 * 0.66666f);
	int ri = clamp(int(r*256), 0, 255);
	int gi = clamp(int(g*256), 0, 255);
	int bi = clamp(int(b*256), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}

template struct Point<FltH>;
template struct Point<FltL>;

// ----------------------------------------------------------------------------

// ***********
// *** Map ***
// ***********

template<typename Flt>
Point<Flt>& Map<Flt>::get(UL x, UL y)
{
	return points[y][x];
}

template<typename Flt>
Flt Map<Flt>::to_xpos(UL x) const
{
	assert(x<new_w);
	Flt fact = x;
	fact /= (Flt)new_w;
	fact -= (Flt)0.5;
	fact *= scale_x;
	fact += center_x;
	return fact;
}

template<typename Flt>
Flt Map<Flt>::to_ypos(UL y) const
{
	assert(y<new_h);
	Flt fact = y;
	fact /= (Flt)new_h;
	fact -= (Flt)0.5;
	fact *= scale_y;
	fact += center_y;
	return fact;
}

template<typename Flt>
void Map<Flt>::generate_init()
{
	UL x,y;
	vfx.clear();
	vfy.clear();
	
	new_w = width; new_h = height;

	for (y=0; y<height; ++y)
		vfy.push_back(to_ypos(y));
	for (x=0; x<width; ++x)
		vfx.push_back(to_xpos(x));

	map_all_done = false;
	points.resize(height);

	for (y=0; y<height; ++y)
	{
		points[y].resize(width);
		const Flt& yld = vfy[y];
		for (x=0; x<width; ++x)
		{
			const Flt& xld = vfx[x];
			get(x,y).init({xld, yld});
		}
	}
}

template<typename Flt>
auto Map<Flt>::generate(UL cap, bool display, bool extrap) -> Status
{
	bool found_one = false;
	bool did_smth = false;

	UL x,y;

	for (y=0; y<height; ++y)
	{
		if (display)
		{
			float f = 100.0f;
			f /= height;
			f *= y;
			std::cout << (int)f << "%\r" << std::flush;
		}
		const Flt& yld = vfy[y];
		for (x=0; x<width; ++x)
		{
			Point<Flt>& p = get(x,y);
			if (p.status != Point<Flt>::calc)
				continue;
			found_one = true;
			if (extrap)
			{
				UL w = width-1, h = height-1;
				if (x && x<w)
				{
					auto& pp = get(x-1,y);
					auto& pn = get(x+1,y);
					if ( (pp.status == Point<Flt>::out) &&
					     (pn.status == Point<Flt>::out) &&
					     (pp.iter == pn.iter) )
					{
						p.status = Point<Flt>::out;
						p.iter = pp.iter;
						p.over = (pp.over + pn.over)/2.0;
						did_smth = true;
						continue;
					}
				}
				if (y && y<h)
				{
					Point<Flt>& pp = get(x,y-1);
					Point<Flt>& pn = get(x,y+1);
					if ( (pp.status == Point<Flt>::out) &&
					     (pn.status == Point<Flt>::out) &&
					     (pp.iter == pn.iter) )
					{
						p.status = Point<Flt>::out;
						p.iter = pp.iter;
						p.over = (pp.over + pn.over)/2.0;
						did_smth = true;
						continue;
					}
				}
			}
			std::complex<Flt> c{vfx[x], yld};
			did_smth = p.docalc(c, p.iter+cap) || did_smth;
		}
	}
	if (!found_one)
		return all_done;
	else
		return did_smth ? was_updated : no_change;
}

template<typename Flt>
Image Map<Flt>::makeimage(float mod, UL upc)
{
	Image img(width, height);
	UL x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			auto& p = get(x,y);
			if (p.status == Point<Flt>::out)
			{
				img.PutPixel(x,y,p.col(mod));
			} else {
				bool clc = p.status==Point<Flt>::calc;
				bool ilu = upc && (p.iter<upc);
				if (clc && ilu)
					img.PutPixel(x,y,{255,255,255});
				else
					img.PutPixel(x,y,{0,0,0});
			}
		}
	}
	return img;
}

static RGB mix(const RGB& p1,const RGB& p2, float f)
{
	assert( (f >= 0.0f) && (f <= 1.0f) );
	const float inv = 1.0f-f;
	float r = p1.r * f + p2.r * inv;
	float g = p1.g * f + p2.g * inv;
	float b = p1.b * f + p2.b * inv;
	int ri = clamp(int(r), 0, 255);
	int gi = clamp(int(g), 0, 255);
	int bi = clamp(int(b), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}

template<typename Flt>
RGB Map<Flt>::extrapolate(float x, float y, float mod)
{
	if (x<0.0f) x=0.0f; if (x>new_w) x=new_w;
	if (y<0.0f) y=0.0f; if (y>new_h) y=new_h;
	float xr = roundf(x);
	float yr = roundf(y);
	float dx = x-xr;
	float dy = y-yr;
	UL x1,x2,y1,y2;
	float fx, fy;
	if (fabs(dx) < 0.05f) {
		x1 = x2 = roundl(x);
		fx = 0.5f;
	} else {
		x1 = floorl(x);
		x2 = ceill(x);
		fx = x2-x;
	}
	if (fabs(dy) < 0.05f) {
		y1 = y2 = roundl(y);
		fy = 0.5f;
	} else {
		y1 = floorl(y);
		y2 = ceill(y);
		fy = y2-y;
	}
	RGB pix_11 = get(x1 , y1).col(mod);
	RGB pix_12 = get(x1 , y2).col(mod);
	RGB pix_21 = get(x2 , y1).col(mod);
	RGB pix_22 = get(x2 , y2).col(mod);
	RGB pix_1 = mix(pix_11, pix_12, fy);
	RGB pix_2 = mix(pix_21, pix_22, fy);
	RGB pix = mix(pix_1, pix_2, fx);
	return pix;
}

template<typename Flt>
void Map<Flt>::generate_init_rest()
{
	points.resize(new_h);
	UL x,y;
	for (y=0; y<new_h; ++y)
		points[y].resize(new_w+3);

	Flt x_start = to_xpos(0);
	Flt x_stop  = to_xpos(new_w-1);
	Flt y_start = to_ypos(0);
	Flt y_stop  = to_ypos(new_h-1);
	Flt x_step  = (x_stop-x_start) / (new_w-1);
	Flt y_step  = (y_stop-y_start) / (new_h-1);

	vfx.resize(new_w); vfy.resize(new_h);
	for (x=0; x<new_w; ++x)
		vfx[x] = (x_start + x_step*x);
	for (y=0; y<new_h; ++y)
		vfy[y] = (y_start + y_step*y);
}

template<typename Flt>
void Map<Flt>::generate_N_init(int n, bool disp)
{
	double zm = (double)zoom_mul;
	double tm = pow(zm, -n);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

	if (disp)
	{
		std::cout << "Old size " << width << "x" << height << std::endl;
		std::cout << "New size " << new_w << "x" << new_h << std::endl;
	}

	generate_init_rest();
}

template<typename Flt>
UL Map<Flt>::generate_N_threaded(int n, UL cap, bool display)
{
	generate_N_init(n, display);

	Updater::Init(new_h*3+4);
	if (display) Updater::Display();

	LineCache<Flt> lc[4] = {
		{ cap, display, *this },
		{ cap,   false, *this },
		{ cap,   false, *this },
		{ cap,   false, *this },
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
		tt[i] = boost::thread{&LineCache<Flt>::execute, lc+i};
	}
	LineCache<Flt>::execute(lc);
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

template<typename Flt>
Image Map<Flt>::makeimage_N(int n, ModFunc mf)
{
	Image img(width, height);
	double sx{scale_x};

	double t0{zoom_mul};
	double tpmn = pow(t0, -n);
	double myw = new_w / tpmn;
	double myh = new_h / tpmn;
	
	double xstart = (new_w-myw) / 2;
	double ystart = (new_h-myh) / 2;

	double xstep = myw / width;
	double ystep = myh / height;

	double mod = mf(sx * tpmn);

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

template<typename Flt>
void Map<Flt>::saveblob(int N, std::ostream& out)
{
	{
		std::uint16_t val;
		val = width;  out.write((char*)&val, 2);
		val = height; out.write((char*)&val, 2);
		val = new_w;  out.write((char*)&val, 2);
		val = new_h;  out.write((char*)&val, 2);
		val = N;      out.write((char*)&val, 2);
	}
	
	UC val = 0;
	UC cnt = 0;
	auto put2 = [&](UC bits)
	{
		val = (val << 2) | bits;
		cnt += 2;
		if (cnt == 8)
		{
			out.write((char*)&val, 1);
			val = cnt = 0;
		}
	};
	
	for (UL y=0; y<new_h; ++y)
	{
		for (UL x=0; x<new_w; ++x)
		{
			auto& pix = get(x,y);
			UC pt;
			switch (pix.pixtype)
			{
				case pt_normal: pt = 0; break;
				case pt_ep_hor: pt = 1; break;
				case pt_ep_ver: pt = 2; break;
				case pt_black:  pt = 3; break;
				default: throw "pixeltype error";
			}
			put2(pt);
		}
	}
	while (cnt) put2(0);
	
	for (UL y=0; y<new_h; ++y)
	{
		for (UL x=0; x<new_w; ++x)
		{
			auto& pix = get(x,y);
			if (pix.pixtype == pt_normal)
				out.write((char*)&pix.over, sizeof(float));
		}
	}
}

template<typename Flt>
void Map<Flt>::setup_dbl(Flt target)
{
	double n = std::log(0.5) / std::log((double)target);
	count_dlb = std::roundl(n);
	double factor = std::pow(0.5, 1.0/n);
	std::cout << "new factor     : " << factor << std::endl;
	std::cout << "new count      : " << count_dlb << std::endl;
	width  = (width  >> 2) << 2;
	height = (height >> 2) << 2;
	new_w = width  * 2;
	new_h = height * 2;
	std::cout << "adjusted size  : " << width << "x" << height << std::endl;
	std::cout << "new size       : " << new_w << "x" << new_h << std::endl;

	generate_init_rest();

	for (UL y=0; y<new_h; ++y)
	{
		const Flt& yld = vfy[y];
		for (UL x=0; x<new_w; ++x)
		{
			const Flt& xld = vfx[x];
			get(x,y).init({xld, yld});
		}
	}

}

template<typename Flt>
int Map<Flt>::generate_dbl(UL cap, bool first, bool display)
{
	Updater::Init(new_h*3+4);
	if (display) Updater::Display();

	LineCache<Flt> lc[4] = {
		{ cap, display, *this, first },
		{ cap,   false, *this, first },
		{ cap,   false, *this, first },
		{ cap,   false, *this, first },
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
		tt[i] = boost::thread{&LineCache<Flt>::execute_dbl, lc+i};
	}
	LineCache<Flt>::execute_dbl(lc);
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
	{
		std::cout << "skipped        : " << sk << " pixels, of wich " << is << " was inside \n";
		std::cout << "effective cap  : " << maxout << "\n";
	}

	return count_dlb;
}

template<typename Flt>
int Map<Flt>::sh_new_xcoord(int oldx)
{
	int hw = new_w/2;
	if (oldx < hw) // left
	{
		int dfc = hw - oldx;
		return hw - 2*dfc;
	} else { // right
		hw -= 1;
		int dfc = oldx - hw;
		return hw + 2*dfc;
	}
}

template<typename Flt>
int Map<Flt>::sh_new_ycoord(int oldy)
{
	int hh = new_h/2;
	if (oldy < hh)
	{
		int dfc = hh - oldy;
		return hh - 2*dfc;
	} else {
		hh -= 1;
		int dfc = oldy - hh;
		return hh + 2*dfc;
	}
}

template<typename Flt>
void Map<Flt>::shuffle_dbl()
{
	generate_init_rest();

	auto copy = points;
	for (UL y=0; y<new_h; ++y)
	{
		const Flt& yld = vfy[y];
		for (UL x=0; x<new_w; ++x)
		{
			const Flt& xld = vfx[x];
			get(x,y).init({xld, yld});
		}
	}

	for (int y=0; y<(int)new_h; ++y)
	{
		auto newy = sh_new_ycoord(y);
		if ((newy<0) || (newy>=(int)new_h)) continue;
		for (int x=0; x<(int)new_w; ++x)
		{
			auto newx = sh_new_xcoord(x);
			if ((newx<0) || (newx>=(int)new_w)) continue;
			const Point<Flt>& src = copy[y][x];
			get(newx,newy) = src;
		}
	}
	//points.swap(copy);
	//using std::swap;
	//swap(width, new_w); swap(height, new_h);
	//makeimage(350,INT_MAX).Save("WorkImage.bmp");
	//swap(width, new_w); swap(height, new_h);
}

template struct Map<FltH>;
template struct Map<FltL>;

// ----------------------------------------------------------------------------

// ***************l
// *** Updater ***
// ***************

std::mutex Updater::lck;
UL Updater::count, Updater::max, Updater::last;

void Updater::Init(UL m)
{
	count = 0;
	max = m;
	last = (UL)-1;
}

void Updater::Tick()
{
	std::lock_guard guard(lck);
	++count;
}

void Updater::Display()
{
	std::lock_guard guard(lck);
	float f = 100.0f;
	f /= max;
	f *= count;
	UL p = (int)f;
	if (p==last) return;
	std::cout << p << "%\r" << std::flush;
	last = p;
}

int Updater::Get()
{
	std::lock_guard guard(lck);
	float f = 100.0f;
	f /= max;
	f *= count;
	return (int)f;
}

// ----------------------------------------------------------------------------

// *****************
// *** LineCache ***
// *****************

template<typename Flt>
LineCache<Flt>::LineCache(UL cap, bool display, Map<Flt>& map, bool first)
	: cap(cap)
	, display(display)
	, first(first)
	, map(map)
	, vfx(map.vfx)
	, vfy(map.vfy)
{}

template<typename Flt>
void LineCache<Flt>::base_init()
{
	eff_cap = 1;
	skip_count = 0;
	inskip = 0;
	n = y_count;
	w = map.new_w;
}

template<typename Flt>
void LineCache<Flt>::init_zero()
{
	for (UL i=0; i<n; ++i)
	{
		UL y = y_start + i;
		Flt& yld = vfy[y];
		for (UL x=0; x<w; ++x)
		{
			auto& p = map.get(x,y);
			std::complex<Flt> c{vfx[x], yld};
			p.init(c);
		}
	}
}

template<typename Flt>
void LineCache<Flt>::init_lim()
{
	xlo = 0;
	xhi = map.new_w-1;
	ylo = y_start;
	yhi = y_start+n-1;
}

template<typename Flt>
void LineCache<Flt>::dc(Point<Flt>& p, const std::complex<Flt>& c)
{
	bool did = p.docalc(c, cap);
	if (did && p.iter>eff_cap)
		eff_cap = p.iter;
}

template<typename Flt>
void LineCache<Flt>::even()
{
	for (UL i=0; i<n; ++i)
	{
		Updater::Tick();
		if (display)
			Updater::Display();
		if (i%2) continue;
		UL y = y_start + i;
		Flt& yld = vfy[y];
		for (UL x=0; x<w; x+=2)
		{
			auto& p = map.get(x,y);
			std::complex<Flt> c{vfx[x], yld};
			dc(p,c);
		}
	}
}

template<typename Flt>
void LineCache<Flt>::odd()
{
	for (UL i=0; i<n; ++i)
	{
		Updater::Tick();
		if (display)
			Updater::Display();
		if (!(i%2)) continue;
		UL y = y_start + i;
		Flt& yld = vfy[y];
		for (UL x=1; x<w; x+=2)
		{
			auto& p = map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter != 1) continue;
			std::complex<Flt> c{vfx[x], yld};
			dc(p,c);
		}
	}
}

template<typename Flt>
bool LineCache<Flt>::ep_xy(UL x, UL y)
{
	auto& p = map.get(x,y);
	if (p.status != Point<Flt>::calc) return false;
	if (p.iter != 1) return false;
	if (x==xlo) return false;
	if (x==xhi) return false;
	if (y==ylo) return false;
	if (y==yhi) return false;
	if (map.get(x-1,y).status != Point<Flt>::in) return false;
	if (map.get(x+1,y).status != Point<Flt>::in) return false;
	if (map.get(x,y-1).status != Point<Flt>::in) return false;
	if (map.get(x,y+1).status != Point<Flt>::in) return false;
	p.status = Point<Flt>::in;
	p.pixtype = pt_black;
	++skip_count;
	++inskip;
	return true;
}

template<typename Flt>
bool LineCache<Flt>::ep_x(UL x, UL y)
{
	auto& p = map.get(x,y);
	if (p.status != Point<Flt>::calc) return false;
	if (p.iter != 1) return false;
	if (x==xlo) return false;
	if (x==xhi) return false;
	auto& pp = map.get(x-1,y);
	if (pp.status != Point<Flt>::out) return false;
	auto& pn = map.get(x+1,y);
	if (pn.status != Point<Flt>::out) return false;
	if (pp.iter != pn.iter) return false;
	p.status = Point<Flt>::out;
	p.iter = pp.iter;
	p.over = (pp.over + pn.over)/2.0;
	p.pixtype = pt_ep_hor;
	++skip_count;
	return true;
}

template<typename Flt>
bool LineCache<Flt>::ep_y(UL x, UL y)
{
	auto& p = map.get(x,y);
	if (p.status != Point<Flt>::calc) return false;
	if (p.iter != 1) return false;
	if (y==ylo) return false;
	if (y==yhi) return false;
	auto& pp = map.get(x,y-1);
	if (pp.status != Point<Flt>::out) return false;
	auto& pn = map.get(x,y+1);
	if (pn.status != Point<Flt>::out) return false;
	if (pp.iter != pn.iter) return false;
	p.status = Point<Flt>::out;
	p.iter = pp.iter;
	p.over = (pp.over + pn.over)/2.0;
	p.pixtype = pt_ep_ver;
	++skip_count;
	return true;
}

template<typename Flt>
bool LineCache<Flt>::setin()
{
	bool foundin = false;

	for (UL i=0; i<n; ++i)
	{
		UL y = y_start + i;
		for (UL x=0; x<w; ++x)
		{
			auto& p = map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter < cap) continue;
			p.status = Point<Flt>::in;
			foundin = true;
		}
	}
	return foundin;
}

template<typename Flt>
void LineCache<Flt>::all()
{
	for (UL i=0; i<n; ++i)
	{
		Updater::Tick();
		if (display)
			Updater::Display();

		UL y = y_start + i;
		for (UL x=0; x<w; ++x)
		{
			auto& p = map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter != 1) continue;
			std::complex<Flt> c{vfx[x], vfy[y]};
			dc(p,c);
		}
	}
}

template<typename Flt>
void LineCache<Flt>::execute(LineCache* lc)
{
	lc->base_init();
	lc->init_zero();
	lc->even();
	lc->init_lim();

	auto all_f = [&](auto f) -> void
	{
		for (UL i=0; i<lc->n; i+=1)
		{
			UL y = lc->y_start + i;
			for (UL x=0; x<lc->w; x+=1)
				f(x,y);
		}
	};

	all_f( [&](UL x, UL y) { lc->ep_x(x,y); } );
	all_f( [&](UL x, UL y) { lc->ep_y(x,y); } );

	lc->odd();

	bool foundin = lc->setin();

	if (foundin)
		all_f( [&](UL x, UL y) { lc->ep_xy(x,y); } );

	lc->all();
}

template<typename Flt>
void LineCache<Flt>::execute_dbl(LineCache* lc)
{
	lc->base_init();

	if (lc->first)
		lc->even();

	lc->init_lim();

	auto all_f = [&](auto f) -> void
	{
		for (UL i=0; i<lc->n; i+=1)
		{
			UL y = lc->y_start + i;
			for (UL x=0; x<lc->w; x+=1)
				f(x,y);
		}
	};

	all_f( [&](UL x, UL y) { lc->ep_x(x,y); } );
	all_f( [&](UL x, UL y) { lc->ep_y(x,y); } );

	if (lc->first)
		lc->odd();
	else
		lc->even();

	bool foundin = lc->setin();

	if (foundin)
		all_f( [&](UL x, UL y) { lc->ep_xy(x,y); } );

	lc->all();
}

template struct LineCache<FltH>;
template struct LineCache<FltL>;


