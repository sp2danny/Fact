
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>

#include <gtk/gtk.h>

#include "graph.h"

typedef std::complex<long double> Cmplx;

Cmplx step(Cmplx c, Cmplx z)
{
	return z*z + c;
}

struct Point
{
	enum { in, calc, out } status = calc;
	unsigned long iter = 0;
	Cmplx z = {0,0};
};

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

/*
cardoid:

p = srqt( (x-1/4)^2 + y^2 )
x < p - 2p^2 + 1/4

bulb:

(x+1)^2 + y^2 < 1/16
*/

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
					points[idx].status = Point::calc;
					points[idx].iter = n;
					points[idx].z = z;
					break;
				}
				if (std::abs(z) > 2.0l)
				{
					points[idx].status = Point::out;
					points[idx].iter = n;
					points[idx].z = z;
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


RGB col(Point p)
{
	auto x = p.iter;
	auto over = abs(p.z) ; // - 2.0l;

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
	unsigned long x,y;
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


gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	//printf("de 1\n"); fflush(stdout);
	gtk_main_quit();
	return TRUE;
}

void gtk_app(Image& img)
{
	GtkWidget* window;
	GtkWidget* image;
	(void)image;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), nullptr);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);

	GdkPixbuf* pbuf = gdk_pixbuf_new_from_data (
		img.data(),
		GDK_COLORSPACE_RGB,
		FALSE,
		8,
		img.Width(),
		img.Height(),
		img.Width()*3,
		nullptr,
		nullptr
	);
	image = gtk_image_new_from_pixbuf (pbuf);
	gtk_container_add (GTK_CONTAINER (window), image);
	gtk_widget_show(image);
	gtk_widget_show(window);
	gtk_main();
}

int main(int argc, char* argv[])
{
	Map m;
	m.width = 640; m.height = 480;
	m.center_x = (argc>=3) ? atof(argv[2]) : -0.666l;
	m.center_y = (argc>=4) ? atof(argv[3]) : 0.0l;
	m.scale_x  = (argc>=5) ? atof(argv[4]) : 3.2l;
	m.scale_y  = (argc>=6) ? atof(argv[5]) : (m.scale_x*3.0/4.0);
	m.generate((argc>=2) ? atoi(argv[1]) : 500 );
	auto img = m.makeimage();
	img.Save("fact.bmp");

	std::cout << "done." << std::endl;

	gtk_init(&argc, &argv);
	gtk_app(img);
	
	//std::cout << min_f << std::endl << max_f << std::endl;
}




