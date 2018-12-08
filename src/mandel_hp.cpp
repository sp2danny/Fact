

#include "mandel_hp.h"

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
			auto idx = to_index(x, y);
			if (points[idx].status != Point::calc)
				continue;
			found_one = true;
			Cmplx c{vfx[x], yld};
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
				auto az = abs(z);
				if (az > two)
				{
					did_smth = true;
					points[idx].status = Point::out;
					points[idx].iter = n;
					points[idx].z = z;
					points[idx].over = az.get_d();
					break;
				}

				if (n<=1)
				{
					// first bulb
					Flt y2 = yld * yld;
					Flt xldp1 = vfx[x]+1.0;
					if (((xldp1*xldp1) + y2) < (Flt)0.0625)
					{
						did_smth = true;
						points[idx].status = Point::in;
						points[idx].iter = n;
						points[idx].z = z;
						break;
					}
					// main cardoid
					Flt xx = vfx[x]-0.25;
					xx *= xx;
					xx += y2;
					Flt p = sqrt(xx);
					if (vfx[x] < (p - 2.0*(p*p) + 0.25))
					{
						did_smth = true;
						points[idx].status = Point::in;
						points[idx].iter = n;
						points[idx].z = z;
						break;
					}
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

RGB col(Point p, float mod)
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

Image Map::makeimage(float mod)
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
			} else {
				img.PutPixel(x,y,{0,0,0});
			}
		}
	}
	return img;
}
/*
void movie_maker(Map& m, UL cap)
{
	Flt zoom_cur = 3.2;
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
		std::sprintf(buff, "./img/m%05lu.bmp", num);
		img.Save(buff);
		zoom_cur *= 0.99;
		if (zoom_cur < zoom_end) break;
		num += 1;
	}
}
*/

void Map::generate_10(UL cap, bool disp)
{
	double zm = zoom_mul.get_d();
	double tm = pow(zm, -10);

	new_w = ceil(width  * tm);
	new_h = ceil(height * tm);

	points.resize(new_w*new_h);

	Flt x_start = to_xpos(0);
	Flt x_stop  = to_xpos(width-1);
	Flt y_start = to_ypos(0);
	Flt y_stop  = to_ypos(height-1);
	Flt x_step  = (x_stop-x_start) / (new_w-1);
	Flt y_step  = (y_stop-y_start) / (new_h-1);

	UL x,y;
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
			auto idx = x + y*new_w;
			Cmplx c{vfx[x], yld};
			Cmplx z = c;
			UL n = 1;
			
			// first bulb
			Flt y2 = yld * yld;
			Flt xldp1 = vfx[x]+1.0;
			if (((xldp1*xldp1) + y2) < (Flt)0.0625)
			{
				points[idx].status = Point::in;
				points[idx].iter = n;
				points[idx].z = z;
				continue;
			}
			// main cardoid
			Flt xx = vfx[x]-0.25;
			xx *= xx;
			xx += y2;
			Flt p = sqrt(xx);
			if (vfx[x] < (p - 2.0*(p*p) + 0.25))
			{
				points[idx].status = Point::in;
				points[idx].iter = n;
				points[idx].z = z;
				continue;
			}
						
			while (true)
			{
				if (n >= cap)
				{
					points[idx].status = Point::calc;
					points[idx].iter = n;
					points[idx].z = z;
					break;
				}
				auto az = abs(z).get_d();
				if (az > two)
				{
					points[idx].status = Point::out;
					points[idx].iter = n;
					points[idx].z = z;
					points[idx].over = az;
					break;
				}

				z = step(c, z);
				++n;
			}
		}
	}
}
/*
float iter(Point p)
{
	static const float ilg2 = 1.0f / log(2.0f);

	float f = p.iter + 1.0f;
	float fover = p.over;
	f -= (float)log(log(fover) * ilg2) * ilg2;
	return f;
}

RGB topix(float f, float mod)
{
	static const float pi2 = 3.1415926536f * 2;
	f = fmod(f, mod);
	f /= mod;
	f *= pi2;
	float r = 0.5f + 0.5f*std::sin(f + pi2 * 0.00000f);
	float g = 0.5f + 0.5f*std::sin(f + pi2 * 0.33333f);
	float b = 0.5f + 0.5f*std::sin(f + pi2 * 0.66666f);
	int ri = clamp(int(r*256), 0, 255);
	int gi = clamp(int(g*256), 0, 255);
	int bi = clamp(int(b*256), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}
*/
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
	RGB pix_11 = col(points[x1 + y1*new_w], mod);
	RGB pix_12 = col(points[x1 + y2*new_w], mod);
	RGB pix_21 = col(points[x2 + y1*new_w], mod);
	RGB pix_22 = col(points[x2 + y2*new_w], mod);
	RGB pix_1 = mix(pix_11, pix_12, fy);
	RGB pix_2 = mix(pix_21, pix_22, fy);
	RGB pix = mix(pix_1, pix_2, fx);
	return pix;
}

Image Map::makeimage_N(int n, ModFunc mf)
{
	Image img(width, height);
	float sx = scale_x.get_d();
	
	
	/*if (n==0)
	{
		UL x,y;
		float yfact = float(new_h) / float(height);
		float xfact = float(new_w) / float(width);
		for (y=0; y<height; ++y)
		{
			float yf = y * yfact;
			for (x=0; x<width; ++x)
			{
				float xf = x * xfact;
				
				img.PutPixel(x,y,extrapolate(xf,yf, mod));
			}
		}
	}
	else*/
	{
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

	}
		
	return img;
}

