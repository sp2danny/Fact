

#include "mandel_hp.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <mutex>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

Flt copysign(const Flt& a,const Flt& b)
{
	bool aneg = a<0;
	bool bneg = b<0;
	if (aneg==bneg)
		return a;
	else
		return -a;
}

using namespace std::literals;

Cmplx step(Cmplx c, Cmplx z)
{
	return z*z + c;
}

bool Point::docalc(const Cmplx& c, UL cap)
{
	UL& n = iter;
	if (n <= 1)
	{
		// first bulb
		double xld = z.real().get_d();
		double yld = z.imag().get_d();
		double y2 = yld * yld;
		double xldp1 = xld+1.0;
		if (((xldp1*xldp1) + y2) < 0.0625)
		{
			status = Point::in;
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
			return true;
		}
	}

	bool did_smth = false;

	while (true)
	{
		if (n >= cap)
			break;

		auto zre = z.real();
		Flt  re_sq = zre * zre;
		auto zim = z.imag();
		Flt  im_sq = zim * zim;
		auto az2 = re_sq.get_d() + im_sq.get_d();
		if (az2 > 4.0)
		{
			did_smth = true;
			status = Point::out;
			over = sqrtf(az2);
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

void Point::init(const Cmplx& c)
{
	status = Point::calc;
	iter = 1;
	z = c;
}

Point& Map::get(UL x, UL y)
{
	return points[y][x];
}

Flt Map::to_xpos(UL x) const
{
	assert(x<width);
	Flt fact = x;
	fact /= (Flt)width;
	fact -= (Flt)0.5;
	fact *= scale_x;
	fact += center_x;
	return fact;
}

Flt Map::to_ypos(UL y) const
{
	assert(y<height);
	Flt fact = y;
	fact /= (Flt)height;
	fact -= (Flt)0.5;
	fact *= scale_y;
	fact += center_y;
	return fact;
}

void Map::generate_init()
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

auto Map::generate(UL cap, bool display, bool extrap) -> Status
{
	bool found_one = false;
	bool did_smth = false;

	UL x,y;

	const Flt two = 2.0;

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
			Point& p = get(x,y);
			if (p.status != Point::calc)
				continue;
			found_one = true;
			if (extrap)
			{
				UL w = width-1, h = height-1;
				if (x && x<w)
				{
					Point& pp = get(x-1,y);
					Point& pn = get(x+1,y);
					if ( (pp.status == Point::out) &&
					     (pn.status == Point::out) &&
					     (pp.iter == pn.iter) )
					{
						p.status = Point::out;
						p.iter = pp.iter;
						p.over = (pp.over + pn.over)/2.0;
						did_smth = true;
						continue;
					}
				}
				if (y && y<h)
				{
					Point& pp = get(x,y-1);
					Point& pn = get(x,y+1);
					if ( (pp.status == Point::out) &&
					     (pn.status == Point::out) &&
					     (pp.iter == pn.iter) )
					{
						p.status = Point::out;
						p.iter = pp.iter;
						p.over = (pp.over + pn.over)/2.0;
						did_smth = true;
						continue;
					}
				}
			}
			Cmplx c{vfx[x], yld};
			did_smth = p.docalc(c, p.iter+cap) || did_smth;
		}
	}
	if (!found_one)
		return all_done;
	else
		return did_smth ? was_updated : no_change;
}

void Map::generate_odd(UL cap)
{
	UL x,y;

	const Flt two = 2.0;

	for (y=0; y<height; y+=2)
	{
		{
			float f = 100.0f;
			f /= height;
			f *= y;
			std::cout << (int)f << "%\r" << std::flush;
		}
		const Flt& yld = vfy[y];
		for (x=0; x<width; x+=2)
		{
			Point& p = get(x,y);
			if (p.status != Point::calc)
				continue;
			Cmplx c{vfx[x], yld};
			p.docalc(c, cap);
		}
	}
}

RGB col(const Point& p, float mod)
{
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

Image Map::makeimage(float mod, UL upc)
{
	Image img(width, height);
	UL x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			auto& p = get(x,y);
			if (p.status == Point::out)
			{
				img.PutPixel(x,y,col(p, mod));
			} else {
				if ((p.status==Point::calc) && upc && (p.iter<upc))
					img.PutPixel(x,y,{255,255,255});
				else
					img.PutPixel(x,y,{0,0,0});
			}
		}
	}
	return img;
}

RGB mix(const RGB& p1,const RGB& p2, float f)
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

RGB Map::extrapolate(float x, float y, float mod)
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
	RGB pix_11 = col(get(x1 , y1), mod);
	RGB pix_12 = col(get(x1 , y2), mod);
	RGB pix_21 = col(get(x2 , y1), mod);
	RGB pix_22 = col(get(x2 , y2), mod);
	RGB pix_1 = mix(pix_11, pix_12, fy);
	RGB pix_2 = mix(pix_21, pix_22, fy);
	RGB pix = mix(pix_1, pix_2, fx);
	return pix;
}

Image Map::makeimage_10_N(int n, ModFunc mf)
{
	Image img(width, height);
	double sx{scale_x};

	++n;
	double t0{zoom_mul};
	double tm  = powf(t0, -1);
	double tn  = powf(t0, -n);
	double t10 = powf(t0, -11); (void)t10;
	double dif = t10-tm;
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

void execute(LineCache* lc)
{
	UL maxout = 1, skipcnt = 0, inskip = 0;
	const Flt two = 2.0;
	UL i, n=lc->y_count;
	UL w = lc->map.new_w;

	for (i=0; i<n; ++i)
	{
		UL y = lc->y_start + i;
		Flt& yld = lc->vfy[y];
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			Cmplx c{lc->vfx[x], yld};
			p.init(c);
		}
	}

	auto dc = [&](Point& p, const Cmplx& c) -> void
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
			Cmplx c{lc->vfx[x], yld};
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
		if (p.status != Point::calc) return false;
		if (p.iter != 1) return false;
		if (x==xlo) return false;
		if (x==xhi) return false;
		if (y==ylo) return false;
		if (y==yhi) return false;
		Point* pp;
		pp = &lc->map.get(x-1,y); if (pp->status != Point::in) return false;
		pp = &lc->map.get(x+1,y); if (pp->status != Point::in) return false;
		pp = &lc->map.get(x,y-1); if (pp->status != Point::in) return false;
		pp = &lc->map.get(x,y+1); if (pp->status != Point::in) return false;
		p.status = Point::in;
		++skipcnt;
		++inskip;
		return true;
	};

	auto ep_x = [&](UL x, UL y) -> bool
	{
		auto& p = lc->map.get(x,y);
		if (p.status != Point::calc) return false;
		if (p.iter != 1) return false;
		if (x==xlo) return false;
		if (x==xhi) return false;
		auto& pp = lc->map.get(x-1,y);
		if (pp.status != Point::out) return false;
		auto& pn = lc->map.get(x+1,y);
		if (pn.status != Point::out) return false;
		if (pp.iter != pn.iter) return false;
		p.status = Point::out;
		p.iter = pp.iter;
		p.over = (pp.over + pn.over)/2.0;
		++skipcnt;
		return true;
	};

	auto ep_y = [&](UL x, UL y) -> bool
	{
		auto& p = lc->map.get(x,y);
		if (p.status != Point::calc) return false;
		if (p.iter != 1) return false;
		if (y==ylo) return false;
		if (y==yhi) return false;
		auto& pp = lc->map.get(x,y-1);
		if (pp.status != Point::out) return false;
		auto& pn = lc->map.get(x,y+1);
		if (pn.status != Point::out) return false;
		if (pp.iter != pn.iter) return false;
		p.status = Point::out;
		p.iter = pp.iter;
		p.over = (pp.over + pn.over)/2.0;
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
			if (p.status != Point::calc) continue;
			if (p.iter != 1) continue;
			Cmplx c{lc->vfx[x], yld};
			dc(p,c);
		}
	}

	for (i=0; i<n; ++i)
	{
		UL y = lc->y_start + i;
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			if (p.status != Point::calc) continue;
			if (p.iter < lc->cap) continue;
			p.status = Point::in;
		}
	}

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
			if (p.status != Point::calc) continue;
			if (p.iter != 1) continue;
			Cmplx c{lc->vfx[x], lc->vfy[y]};
			dc(p,c);
		}
	}

	if (maxout > lc->eff_cap)
		lc->eff_cap = maxout;
	lc->skip_count = skipcnt;
	lc->inskip = inskip;
}

void Map::generate_10_init(UL )
{
	double zm = zoom_mul.get_d();
	double tm = pow(zm, -10);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

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

UL Map::generate_10_threaded(UL cap, bool display)
{
	generate_10_init(cap);

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

UL Map::generate_10(UL cap, bool display)
{
	generate_10_init(cap);

	Updater::Init(new_h*3+1);
	if (display) Updater::Display();

	LineCache lc = { cap, 0, 0, 0, display, 0, new_h, *this, vfx, vfy };

	execute(&lc);

	Updater::Tick();
	if (display) Updater::Display();

	if (display)
		std::cout << "skipped        : " << lc.skip_count << " pixels, of wich " << lc.inskip << " was inside \n";

	return lc.eff_cap;

}




void Map::generate_25_init(UL)
{
	double zm = zoom_mul.get_d();
	double tm = pow(zm, -25);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

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

UL Map::generate_25_threaded(UL cap, bool display)
{
	generate_25_init(cap);

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

Image Map::makeimage_25_N(int n, ModFunc mf)
{
	Image img(width, height);
	double sx{scale_x};

	++n;
	double t0{zoom_mul};
	double tm  = powf(t0, -1);
	double tn  = powf(t0, -n);
	double t25 = powf(t0, -26);
	double dif = t25-tm;
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
