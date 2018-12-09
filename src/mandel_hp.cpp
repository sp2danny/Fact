

#include "mandel_hp.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <thread>

Cmplx step(Cmplx c, Cmplx z)
{
	return z*z + c;
}

bool Point::docalc(const Cmplx& c, UL cap)
{
	UL& n = iter;
	if (n<=1)
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
		double xx = xld-0.25;
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
	map_all_done = false;
	points.resize(height);
	UL x,y;
		
	for (y=0; y<height; ++y)
	{
		points[y].resize(width);
		Flt yld = to_ypos(y);
		for (x=0; x<width; ++x)
		{
			Flt xld = to_xpos(x);
			get(x,y).init({xld, yld});
		}
	}
}

auto Map::generate(UL cap, bool display) -> Status
{
	bool found_one = false;
	bool did_smth = false;

	UL x,y;
	std::vector<Flt> vfx;
	for (x=0; x<width; ++x)
		vfx.push_back(to_xpos(x));
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
		Flt yld = to_ypos(y);
		for (x=0; x<width; ++x)
		{
			Point& p = get(x,y);
			if (p.status != Point::calc)
				continue;
			found_one = true;
			Cmplx c{vfx[x], yld};
			did_smth = p.docalc(c, p.iter+cap);
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
	std::vector<Flt> vfx;
	for (x=0; x<width; ++x)
		vfx.push_back(to_xpos(x));
	const Flt two = 2.0;

	for (y=0; y<height; y+=2)
	{
		{
			float f = 100.0f;
			f /= height;
			f *= y;
			std::cout << (int)f << "%\r" << std::flush;
		}
		Flt yld = to_ypos(y);
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

	float fover = p.over;
	f -= (float)log(log(fover) * ilg2) * ilg2 / mod;
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
			//auto idx = to_index(x, y);
			auto& p = get(x,y); //points[idx];
			if (p.status == Point::out)
			{
				img.PutPixel(x,y,col(p, mod));
			} else {
				if (upc && (p.iter<upc))
					img.PutPixel(x,y,{255,255,255});
				else
					img.PutPixel(x,y,{0,0,0});
			}
		}
	}
	return img;
}

UL Map::generate_10(UL cap, bool disp)
{
	double zm = zoom_mul.get_d();
	double tm = pow(zm, -10);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

	UL x,y;

	points.resize(new_h);
	for (y=0; y<new_h; ++y)
		points[y].resize(new_w+3);

	Flt x_start = to_xpos(0);
	Flt x_stop  = to_xpos(width-1);
	Flt y_start = to_ypos(0);
	Flt y_stop  = to_ypos(height-1);
	Flt x_step  = (x_stop-x_start) / (new_w-1);
	Flt y_step  = (y_stop-y_start) / (new_h-1);

	UL maxout = 1;
	std::vector<Flt> vfx, vfy;
	for (x=0; x<new_w; ++x)
		vfx.push_back(x_start + x_step*x);
	for (y=0; y<new_h; ++y)
		vfy.push_back(y_start + y_step*y);
	const Flt two = 2.0;

	for (y=0; y<new_h; ++y)
	{
		if (disp)
		{
			float f = 100.0f;
			f /= new_h;
			f *= y;
			std::cout << (int)f << "%\r" << std::flush;
		}
		Flt yld = vfy[y];
		for (x=0; x<new_w; ++x)
		{
			//auto idx = x + y*new_w;
			auto& p = get(x,y);
			Cmplx c{vfx[x], yld};
			p.init(c);
			bool out = p.docalc(c, cap);
			if (out && p.iter>maxout) maxout=p.iter;
		}
	}
	return maxout;
}

RGB mix(const RGB& p1,const RGB& p2, float f)
{
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
	UL x1,x2,y1,y2; float fx, fy;
	if (fabs(dx)<0.05f) {
		x1 = x2 = roundl(x);
		fx = 0.5f;
	} else {
		x1 = floorl(x);
		x2 = ceill(x);
		fx = x2-x;
	}
	if (fabs(dy)<0.05f) {
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

Image Map::makeimage_N(int n, ModFunc mf)
{
	Image img(width, height);
	float sx = scale_x.get_d();
	
	++n;
	float t0  = (float)zoom_mul.get_d();
	float tm  = powf(t0, -1);
	float tn  = powf(t0, -n);
	float t10 = powf(t0, -11); (void)t10;
	float dif = t10-tm;
	float per = (tn-tm)/dif;
	
	float mod = mf(sx/tn);

	float xstart = (new_w - width)/2;
	xstart *= per;
	float ystart = (new_h - height)/2;
	ystart *= per;
	float xstep = float(new_w) / float(width);
	xstep = 1.0f + (xstep-1.0f)*(1.0f-per);
	float ystep = float(new_h) / float(height);
	ystep = 1.0f + (ystep-1.0f)*(1.0f-per);
	
	UL x,y;
	for (y=0; y<height; ++y)
	{
		float yf = ystart + y * ystep;
		for (x=0; x<width; ++x)
		{
			float xf = xstart + x * xstep;
			img.PutPixel(x,y,extrapolate(xf,yf, mod));
		}
	}
		
	return img;
}

void execute(LineCache* lc)
{
	UL maxout = 1, skipcnt = 0;
	const Flt two = 2.0;

	for (UL y : lc->scanlines)
	{
		if (lc->display)
		{
			float f = 100.0f;
			f /= lc->map.new_h;
			f *= y;
			std::cout << (int)f << "%\r" << std::flush;
		}
		Flt yld = lc->vfy[y];
		UL w = lc->map.new_w;
		UL l = w-1;
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			Cmplx c{lc->vfx[x], yld};
			p.init(c);
		}

		auto dc = [&](Point& p, const Cmplx& c) -> void
		{
			bool did = p.docalc(c, lc->cap);
			if (did && p.iter>maxout) maxout=p.iter;			
		};

		for (UL x=0; x<w; x+=2)
		{
			auto& p = lc->map.get(x,y);
			Cmplx c{lc->vfx[x], yld};
			dc(p,c);
		}
		for (UL x=0; x<w; ++x)
		{
			auto& p = lc->map.get(x,y);
			if (p.status != Point::calc) continue;
			if (p.iter != 1) continue;
			Cmplx c{lc->vfx[x], yld};
			if (x==0) { p.docalc(c,lc->cap); continue; }
			if (x==l) { p.docalc(c,lc->cap); continue; }
			auto& pp = lc->map.get(x-1,y);
			if (pp.status != Point::out) { dc(p,c); continue; }
			auto& pn = lc->map.get(x+1,y);
			if (pn.status != Point::out) { dc(p,c); continue; }
			if (pp.iter != pn.iter) { dc(p,c); continue; }
			p.status = Point::out;
			p.iter = pp.iter;
			p.over = (pp.over + pn.over)/2.0;
			++skipcnt;
		}

	}

	if (maxout > lc->eff_cap)
		lc->eff_cap = maxout;
	lc->skip_count = skipcnt;
}

UL Map::generate_10_threaded(UL cap, bool display)
{
	double zm = zoom_mul.get_d();
	double tm = pow(zm, -10);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

	UL x,y;

	points.resize(new_h);
	for (y=0; y<new_h; ++y)
		points[y].resize(new_w+3);

	Flt x_start = to_xpos(0);
	Flt x_stop  = to_xpos(width-1);
	Flt y_start = to_ypos(0);
	Flt y_stop  = to_ypos(height-1);
	Flt x_step  = (x_stop-x_start) / (new_w-1);
	Flt y_step  = (y_stop-y_start) / (new_h-1);

	std::vector<Flt> vfx, vfy;
	for (x=0; x<new_w; ++x)
		vfx.push_back(x_start + x_step*x);
	for (y=0; y<new_h; ++y)
		vfy.push_back(y_start + y_step*y);

	LineCache lc[4] = {
		{ cap, 0, 0, display, {}, *this, vfx, vfy },
		{ cap, 0, 0,   false, {}, *this, vfx, vfy },
		{ cap, 0, 0,   false, {}, *this, vfx, vfy },
		{ cap, 0, 0,   false, {}, *this, vfx, vfy },
	};

	int i = 0;
	for (y=0; y<new_h; ++y)
	{
		lc[i].scanlines.push_back(y);
		i = (i+1) % 4;
	}

	UL maxout = 0;
	std::thread tt[4];
	for (i=1; i<4; ++i)
	{
		tt[i] = std::thread{&execute, lc+i};
	}
	execute(lc);
	for (i=1; i<4; ++i)
	{
		tt[i].join();
	}
	UL sk = 0;
	for (i=0; i<4; ++i)
	{
		if (lc[i].eff_cap > maxout)
			maxout = lc[i].eff_cap;
		sk += lc[i].skip_count;
	}
	
	if (display)
		std::cout << "skipped        : " << sk << " pixels  \n";

	return maxout;
}
