
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>

#include "graph.h"

typedef std::complex<long double> Cmplx;

Cmplx step(Cmplx c, Cmplx z)
{
	return z*z + c;
}

typedef unsigned long Point;

struct Map
{
	unsigned long width, height;
	long double scale_x, scale_y;
	long double center_x, center_y;
	std::vector<Point> points;
	std::size_t to_index(unsigned long x, unsigned long y) const;
	long double to_xpos(unsigned long x) const;
	long double to_ypos(unsigned long y) const;
	void generate(unsigned long cap);
	Image makeimage();
};

std::size_t Map::to_index(unsigned long x, unsigned long y) const
{
	assert(x<width);
	assert(y<height);
	std::size_t idx = y;
	idx *= width;
	idx += x;
	return idx;
}

long double Map::to_xpos(unsigned long x) const
{
	assert(x<width);
	long double fact = x;
	fact /= (long double)width;
	fact -= 0.5l;
	return fact * scale_x + center_x;
}

long double Map::to_ypos(unsigned long y) const
{
	assert(y<height);
	long double fact = y;
	fact /= (long double)height;
	fact -= 0.5l;
	return fact * scale_y + center_y;
}

void Map::generate(unsigned long cap)
{
	points.resize(width*height);
	unsigned long x,y;
	for (y=0; y<height; ++y)
	{
		long double yld = to_ypos(y);
		for (x=0; x<width; ++x)
		{
			long double xld = to_xpos(x);
			Cmplx c{xld, yld};
			Cmplx z = c;
			unsigned long n = 1;
			auto idx = to_index(x, y);
			while (true)
			{
				if (n >= cap)
				{
					points[idx] = 0;
					break;
				}
				if (std::abs(z) > 2.1l)
				{
					points[idx] = n;
					break;
				}
				z = step(c, z);
				++n;
			}
		}
	}
}

template<typename T>
T clamp(T val, T min, T max)
{
	if (val<min) val=min;
	if (val>max) val=max;
	return val;
}

RGB col(unsigned long x)
{
	static const float pi2 = 3.1415926536 * 2;
	float f = pi2 * (x % 250) / 250.0f;
	float r = 0.5f + 0.5f*std::sin(f + pi2 * 0.000f);
	float g = 0.5f + 0.5f*std::sin(f + pi2 * 0.333f);
	float b = 0.5f + 0.5f*std::sin(f + pi2 * 0.666f);
	int ri = clamp(int(r*255), 0, 255);
	int gi = clamp(int(g*255), 0, 255);
	int bi = clamp(int(b*255), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}

Image Map::makeimage()
{
	Image img(width, height);
	unsigned long x,y;
	for (y=0; y<height; ++y)
	{
		for (x=0; x<width; ++x)
		{
			auto idx = to_index(x, y);
			auto p = points[idx];
			if (p)
			{
				img.PutPixel(width-x-1,y,col(p));
			} else {
				img.PutPixel(width-x-1,y,{0,0,0});
			}
		}
	}
	return img;
}

int main(int argc, char* argv[])
{
	Map m;
	m.width = 640; m.height = 480;
	m.center_x = (argc>=2) ? atof(argv[1]) : -0.5l;
	m.center_y = (argc>=3) ? atof(argv[2]) : 0.0l;
	m.scale_x  = (argc>=4) ? atof(argv[3]) : 3.2l;
	m.scale_y  = (argc>=5) ? atof(argv[4]) : (m.scale_x*24.0/32.0);
	m.generate(1500);
	m.makeimage().Save("fact.bmp");

	std::cout << "done." << std::endl;
}


