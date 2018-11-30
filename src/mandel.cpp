
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
	for (y=0; y<height; ++y)
	{
		Flt yld = to_ypos(y);
		for (x=0; x<width; ++x)
		{
			Flt xld = to_xpos(x);
			Cmplx z{xld, yld};
			points[idx].status = Point::calc;
			points[idx].iter = 1;
			points[idx].z = z;
			++idx;
		}
	}
}

auto Map::generate(UL cap) -> Status
{
	bool found_one = false;
	bool did_smth = false;

	UL x,y;
	for (y=0; y<height; ++y)
	{
		Flt yld = to_ypos(y);
		for (x=0; x<width; ++x)
		{
			auto idx = to_index(x, y);
			if (points[idx].status != Point::calc)
				continue;
			found_one = true;
			Flt xld = to_xpos(x);
			Cmplx c{xld, yld};
			Cmplx z = points[idx].z;
			UL n = points[idx].iter;
			while (true)
			{
				if (n >= cap)
				{
					points[idx].iter = n;
					points[idx].z = z;
					break;
				}
				if (std::abs(z) > 2.0l)
				{
					did_smth = true;
					points[idx].status = Point::out;
					points[idx].iter = n;
					points[idx].z = z;
					break;
				}

				// first bulb
				Flt y2 = std::pow(yld, 2);
				if ((std::pow(xld+1.0l, 2.0l) + y2) < 0.0625l)
				{
					did_smth = true;
					points[idx].status = Point::in;
					points[idx].iter = n;
					if (n>1)
						std::cout << "found first bulb by secondary means" << std::endl << std::flush;
					points[idx].z = z;
					break;
				}
				// main cardoid
				Flt p = std::sqrt( std::pow(xld-0.25l, 2.0l) + y2 );
				if (xld < (p - 2.0l*std::pow(p, 2.0l) + 0.25l))
				{
					did_smth = true;
					points[idx].status = Point::in;
					points[idx].iter = n;
					if (n>1)
						std::cout << "found main cardoid by secondary means" << std::endl << std::flush;
					points[idx].z = z;
					break;
				}

				z = step(c, z);
				++n;
			}
		}
	}
	if (!found_one)
		return all_done;
	else
		return did_smth ? was_updated : no_change;
}

RGB col(Point p)
{
	auto x = p.iter;
	auto over = abs(p.z) ;

	static const float pi2 = 3.1415926536f * 2;
	static const float ilg2 = 1.0f / log(2.0f);
	float f = ((x % 225) + 1) / 225.0f;

	f -= log(log(over) * ilg2) * ilg2 / 225.0;
	f *= pi2;
	float r = 0.5f + 0.5f*std::sin(f + pi2 * 0.00000f);
	float g = 0.5f + 0.5f*std::sin(f + pi2 * 0.33333f);
	float b = 0.5f + 0.5f*std::sin(f + pi2 * 0.66666f);
	int ri = clamp(int(r*256), 0, 255);
	int gi = clamp(int(g*256), 0, 255);
	int bi = clamp(int(b*256), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}

Image Map::makeimage()
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
				img.PutPixel(x,y,col(p));
			} else {
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
		auto img = m.makeimage();
		std::sprintf(buff, "./img/m%05lu.bmp", num);
		img.Save(buff);
		zoom_cur *= 0.99l;
		if (zoom_cur < zoom_end) break;
		num += 1;
	}
}

