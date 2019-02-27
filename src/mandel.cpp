
#include "mandel.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <cmath>

Cmplx step(Cmplx c, Cmplx z)
{
	return z*z + c;
}

std::size_t Map::to_index(UL x, UL y) const
{
	assert(x<width);
	assert(y<height);
	std::size_t idx = y;
	idx *= width;
	idx += x;
	return idx;
}

Flt Map::to_xpos(UL x) const
{
	assert(x<width);
	Flt fact = x;
	fact /= (Flt)width;
	fact -= 0.5l;
	return fact * scale_x + center_x;
}

Flt Map::to_ypos(UL y) const
{
	assert(y<height);
	Flt fact = y;
	fact /= (Flt)height;
	fact -= 0.5l;
	return fact * scale_y + center_y;
}

void Map::generate_init()
{
	map_all_done = false;
	points.resize(width*height);
	UL x,y, idx = 0;
	xlds.clear(); xlds.reserve(width);
	for (x=0; x<width; ++x)
		xlds.push_back(to_xpos(x));
	ylds.clear(); ylds.reserve(height);
	for (y=0; y<height; ++y)
		ylds.push_back(to_ypos(y));
	for (y=0; y<height; ++y)
	{
		Flt& yld = ylds[y];
		for (x=0; x<width; ++x)
		{
			Flt& xld = xlds[x];
			Cmplx z{xld, yld};
			idx = (UL)to_index(x,y);
			points[idx].status = Point::calc;
			points[idx].iter = 1;
			points[idx].z = z;
			++idx;
		}
	}
}

bool Map::do_one(UL x, UL y, UL cap)
{
	auto idx = to_index(x, y);
	Point& p = points[idx];
	if (p.status != Point::calc)
		return false;

	UL n = p.iter;
	if (n >= cap)
		return false;
	Flt& xld = xlds[x];
	Flt& yld = ylds[y];
	Cmplx c{xld, yld};
	Cmplx z = p.z;
	if (n == 1)
	{
		// first bulb
		Flt y2 = std::pow(yld, 2);
		if ((std::pow(xld+1.0l, 2.0l) + y2) < 0.0625l)
		{
			p.status = Point::in;
			p.iter = n;
			p.z = z;
			return true;
		}
		// main cardoid
		Flt pt = std::sqrt( std::pow(xld-0.25l, 2.0l) + y2 );
		if (xld < (pt - 2.0l*std::pow(pt, 2.0l) + 0.25l))
		{
			p.status = Point::in;
			p.iter = n;
			p.z = z;
			return true;
		}
	}

	while (true)
	{
		if (n >= cap)
		{
			p.iter = cap;
			p.z = z;
			return false;
		}

		auto zre = z.real();
		const Flt  re_sq = zre * zre;
		auto zim = z.imag();
		const Flt  im_sq = zim * zim;
		const auto az2 = re_sq + im_sq;
		if (az2 > 4.0)
		{
			p.status = Point::out;
			p.over = sqrtf((float)az2);
			p.iter = n;
			return true;
		}

		z.real(re_sq-im_sq);
		auto ab = zre * zim;
		z.imag(ab+ab);
		z += c;
		++n;
	}

}

void Map::dothething(UL startx,UL starty)
{
	UL x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			UL idx = (UL)to_index(x,y);
			points[idx].visited = false;
		}
	}
	x = startx; y = starty;
	// part one : find edge
	
}

auto Map::generate(UL cap, bool isfinal) -> Status
{
	bool found_one = false;
	bool did_smth = false;
	
	UL x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			auto idx = to_index(x, y);
			Point& p = points[idx];
			if (p.status != Point::calc)
				continue;
			found_one = true;
			did_smth = do_one(x,y,cap) || did_smth;
			if (isfinal && (p.status == Point::calc) && (p.iter == cap))
			{
				dothething(x, y);
			}
		}
	}
	if (!found_one)
		return all_done;
	else
		return did_smth ? was_updated : no_change;
}

RGB col(Point p, float mod)
{
	auto x = p.iter;

	static const float pi2 = 3.1415926536f * 2;
	static const float ilg2 = 1.0f / log(2.0f);
	float f = fmod((float)x, mod) + 1.0f;
	f /= mod;

	f -= (float)log(log((float)p.over) * ilg2) * ilg2 / mod;
	f *= pi2;
	float r = 0.5f + 0.5f*std::sin(f + pi2 * 0.00000f);
	float g = 0.5f + 0.5f*std::sin(f + pi2 * 0.33333f);
	float b = 0.5f + 0.5f*std::sin(f + pi2 * 0.66666f);
	int ri = clamp(int(r*256), 0, 255);
	int gi = clamp(int(g*256), 0, 255);
	int bi = clamp(int(b*256), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}

Image Map::makeimage(float mod, UL fuc)
{
	Image img(width, height);
	UL x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			auto idx = to_index(x, y);
			auto p = points[idx];
			if (p.status == Point::out)
			{
				img.PutPixel(x,y,col(p, mod));
			}
			else if (p.status == Point::in)
			{
				img.PutPixel(x,y,{0,0,0});
			}
			else if (fuc && p.iter < fuc)
			{
				img.PutPixel(x,y,{255,255,255});
			}
			else
			{
				img.PutPixel(x,y,{0,0,0});			
			}
		}
	}
	return img;
}

void movie_maker(Map& m, UL cap)
{
	Flt zoom_cur = 3.2l;
	Flt zoom_end = m.scale_x;
	Flt xy_factor = m.scale_x / m.scale_y;
	UL num = 1;
	char buff[1024];
	while (true)
	{
		m.scale_x = zoom_cur;
		m.scale_y = zoom_cur / xy_factor;
		m.generate_init();
		m.generate(cap);
		auto img = m.makeimage(225.f);
		std::sprintf(buff, "./img/m%05u.bmp", num);
		img.Save(buff);
		zoom_cur *= 0.99l;
		if (zoom_cur < zoom_end) break;
		num += 1;
	}
}

