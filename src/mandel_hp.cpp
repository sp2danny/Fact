
#include "mandel_hp.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <mutex>

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
	assert(x<width);
	Flt fact = x;
	fact /= (Flt)width;
	fact -= (Flt)0.5;
	fact *= scale_x;
	fact += center_x;
	return fact;
}

template<typename Flt>
Flt Map<Flt>::to_ypos(UL y) const
{
	assert(y<height);
	Flt fact = y;
	fact /= (Flt)height;
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
				if ((p.status==Point<Flt>::calc) && upc && (p.iter<upc))
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
		tt[i] = boost::thread{&execute<Flt>, lc+i};
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

template struct Map<FltH>;
template struct Map<FltL>;

// ----------------------------------------------------------------------------

// ***************
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
void execute(LineCache<Flt>* lc)
{
	UL maxout = 1, skipcnt = 0, inskip = 0;
	UL i, n=lc->y_count;
	UL w = lc->map.new_w;

	for (i=0; i<n; ++i)
	{
		UL y = lc->y_start + i;
		Flt& yld = lc->vfy[y];
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			std::complex<Flt> c{lc->vfx[x], yld};
			p.init(c);
		}
	}

	auto dc = [&](Point<Flt>& p, const std::complex<Flt>& c) -> void
	{
		bool did = p.docalc(c, lc->cap);
		if (did && p.iter>maxout) maxout=p.iter;
	};

	for (i=0; i<n; ++i)
	{
		Updater::Tick();
		if (lc->display)
			Updater::Display();
		if (i%2) continue;
		UL y = lc->y_start + i;
		Flt& yld = lc->vfy[y];
		for (UL x=0; x<w; x+=2)
		{
			auto& p = lc->map.get(x,y);
			std::complex<Flt> c{lc->vfx[x], yld};
			dc(p,c);
		}
	}
	
	UL xlo = 0;
	UL xhi = lc->map.new_w-1;
	UL ylo = lc->y_start;
	UL yhi = lc->y_start+n-1;

	auto ep_xy = [&](UL x, UL y) -> bool
	{
		auto& p = lc->map.get(x,y);
		if (p.status != Point<Flt>::calc) return false;
		if (p.iter != 1) return false;
		if (x==xlo) return false;
		if (x==xhi) return false;
		if (y==ylo) return false;
		if (y==yhi) return false;
		Point<Flt>* pp;
		pp = &lc->map.get(x-1,y); if (pp->status != Point<Flt>::in) return false;
		pp = &lc->map.get(x+1,y); if (pp->status != Point<Flt>::in) return false;
		pp = &lc->map.get(x,y-1); if (pp->status != Point<Flt>::in) return false;
		pp = &lc->map.get(x,y+1); if (pp->status != Point<Flt>::in) return false;
		p.status = Point<Flt>::in;
		p.pixtype = pt_black;
		++skipcnt;
		++inskip;
		return true;
	};

	auto ep_x = [&](UL x, UL y) -> bool
	{
		auto& p = lc->map.get(x,y);
		if (p.status != Point<Flt>::calc) return false;
		if (p.iter != 1) return false;
		if (x==xlo) return false;
		if (x==xhi) return false;
		auto& pp = lc->map.get(x-1,y);
		if (pp.status != Point<Flt>::out) return false;
		auto& pn = lc->map.get(x+1,y);
		if (pn.status != Point<Flt>::out) return false;
		if (pp.iter != pn.iter) return false;
		p.status = Point<Flt>::out;
		p.iter = pp.iter;
		p.over = (pp.over + pn.over)/2.0;
		p.pixtype = pt_ep_hor;
		++skipcnt;
		return true;
	};

	auto ep_y = [&](UL x, UL y) -> bool
	{
		auto& p = lc->map.get(x,y);
		if (p.status != Point<Flt>::calc) return false;
		if (p.iter != 1) return false;
		if (y==ylo) return false;
		if (y==yhi) return false;
		auto& pp = lc->map.get(x,y-1);
		if (pp.status != Point<Flt>::out) return false;
		auto& pn = lc->map.get(x,y+1);
		if (pn.status != Point<Flt>::out) return false;
		if (pp.iter != pn.iter) return false;
		p.status = Point<Flt>::out;
		p.iter = pp.iter;
		p.over = (pp.over + pn.over)/2.0;
		p.pixtype = pt_ep_ver;
		++skipcnt;
		return true;
	};

	auto all_ep_x = [&]() -> void
	{
		for (i=0; i<n; i+=1)
		{
			UL y = lc->y_start + i;
			for (UL x=0; x<w; x+=1)
				ep_x(x,y);
		}
	};

	auto all_ep_y = [&]() -> void
	{
		for (i=0; i<n; i+=1)
		{
			UL y = lc->y_start + i;
			for (UL x=0; x<w; x+=1)
				ep_y(x,y);
		}
	};
	
	auto all_ep_xy = [&]() -> void
	{
		for (i=0; i<n; i+=1)
		{
			UL y = lc->y_start + i;
			for (UL x=0; x<w; x+=1)
				ep_xy(x,y);
		}
	};

	all_ep_x(); all_ep_y();

	for (i=0; i<n; ++i)
	{
		Updater::Tick();
		if (lc->display)
			Updater::Display();
		if (!(i%2)) continue;
		UL y = lc->y_start + i;
		Flt& yld = lc->vfy[y];
		for (UL x=1; x<w; x+=2)
		{
			auto& p = lc->map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter != 1) continue;
			std::complex<Flt> c{lc->vfx[x], yld};
			dc(p,c);
		}
	}

	bool foundin = false;

	for (i=0; i<n; ++i)
	{
		UL y = lc->y_start + i;
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter < lc->cap) continue;
			p.status = Point<Flt>::in;
			foundin = true;
		}
	}

	if (foundin)
		all_ep_xy();

	for (i=0; i<n; ++i)
	{
		Updater::Tick();
		if (lc->display)
			Updater::Display();

		UL y = lc->y_start + i;
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			if (p.status != Point<Flt>::calc) continue;
			if (p.iter != 1) continue;
			std::complex<Flt> c{lc->vfx[x], lc->vfy[y]};
			dc(p,c);
		}
	}

	if (maxout > lc->eff_cap)
		lc->eff_cap = maxout;
	lc->skip_count = skipcnt;
	lc->inskip = inskip;
}

template void execute<FltH>(LineCache<FltH>*);
template void execute<FltL>(LineCache<FltL>*);


