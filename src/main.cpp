
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <iomanip>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "graph.h"
#include "cmdline.h"

cmdline cmd;

typedef long double Flt;

typedef std::complex<Flt> Cmplx;

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
	Flt scale_x, scale_y;
	Flt center_x, center_y;
	bool map_all_done = false;
	std::vector<Point> points;
	std::size_t to_index(unsigned long x, unsigned long y) const;
	Flt to_xpos(unsigned long x) const;
	Flt to_ypos(unsigned long y) const;
	void generate_init();
	enum Status { all_done, was_updated, no_change };
	Status generate(unsigned long cap);
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

Flt Map::to_xpos(unsigned long x) const
{
	assert(x<width);
	Flt fact = x;
	fact /= (Flt)width;
	fact -= 0.5l;
	return fact * scale_x + center_x;
}

Flt Map::to_ypos(unsigned long y) const
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
	unsigned long x,y, idx = 0;
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

auto Map::generate(unsigned long cap) -> Status
{
	bool found_one = false;
	bool did_smth = false;

	unsigned long x,y;
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
			unsigned long n = points[idx].iter;
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
				Flt y2 = pow(yld, 2);
				if ((pow(xld+1.0l, 2.0l) + y2) < 0.0625l)
				{
					did_smth = true;
					points[idx].status = Point::in;
					points[idx].iter = 0;
					points[idx].z = z;
					break;
				}
				// main cardoid
				Flt p = sqrt( pow(xld-0.25l, 2.0l) + y2 );
				if (xld < (p - 2.0l*pow(p, 2.0l) + 0.25l))
				{
					did_smth = true;
					points[idx].status = Point::in;
					points[idx].iter = 0;
					points[idx].z = z;
					break;
				}

				z = step(c, z);
				++n;
			}
		}
	}
	if (!found_one) return all_done;
	else return did_smth ? was_updated : no_change;
}

// enum Status { all_done, was_updated, no_change };

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

Map m;
Image img;

unsigned long update_cap = 100;
unsigned long update_step = 10;
float zoom_step = 2;

gboolean idle_func(gpointer data)
{
	if (m.map_all_done) return TRUE;

	update_step += (update_step/10);
	update_cap += update_step;
	
	auto res = m.generate(update_cap);
	if (res==Map::all_done)
	{
		m.map_all_done = true;
		return TRUE;
	}
	img = m.makeimage();
	
	[[maybe_unused]]
	GtkImage* image = (GtkImage*)data;
	
	[[maybe_unused]]
	GdkPixbuf* pbuf = gdk_pixbuf_new_from_data(
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

	gtk_image_set_from_pixbuf(image, pbuf);
	
	//std::cout << "refreshed at " << update_cap << "\r" << std::flush;

	return TRUE;
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

void mk_img(GtkImage* image)
{
	m.generate_init();
	m.generate(update_cap);
	img = m.makeimage();

	[[maybe_unused]]
	GdkPixbuf* pbuf = gdk_pixbuf_new_from_data(
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

	gtk_image_set_from_pixbuf(image, pbuf);
}

gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	(void)widget;
	(void)data;
	switch (event->keyval)
	{
	case GDK_s:
		img.Save("fact.bmp");
		break;
	case GDK_z:
		zoom_step += 1.0l;
		break;
	case GDK_x:
		zoom_step -= 1.0l;
		break;
	case GDK_space:
		m.scale_x /= zoom_step;
		m.scale_y /= zoom_step;
		mk_img((GtkImage*)data);
		break;
	case GDK_p:
		std::cout << std::setprecision(20);
		std::cout << "center-x     : " << m.center_x  << std::endl;
		std::cout << "center-y     : " << m.center_y  << std::endl;
		std::cout << "scale-x      : " << m.scale_x   << std::endl;
		std::cout << "scale-y      : " << m.scale_y   << std::endl;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "update step  : " << update_step << std::endl;
		std::cout << "zoom step    : " << zoom_step   << std::endl;
		break;
	}

	return TRUE;
}

gboolean button_press(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	if (event->type != GDK_BUTTON_PRESS)
		return TRUE;
	
    if (event->button == 3)
    {
       	m.scale_x *= zoom_step;
		m.scale_y *= zoom_step;
    }
	
    if (event->button == 1)
	{
		long double ldx = m.to_xpos(event->x);
		long double ldy = m.to_ypos(event->y);

		m.center_x = ldx;
		m.center_y = ldy;
		m.scale_x /= zoom_step;
		m.scale_y /= zoom_step;
	}
	
	update_cap = 100;
	update_step = 10;
	
	mk_img((GtkImage*)data);
	return TRUE;
}

void gtk_app()
{
	GtkWidget* window;
	GtkWidget* image;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event",    G_CALLBACK(delete_event), nullptr);

	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	GtkWidget* eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(window), eventbox);
	
	GdkPixbuf* pbuf = gdk_pixbuf_new_from_data(
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
	image = gtk_image_new_from_pixbuf(pbuf);

	g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(key_press), image);
	gtk_widget_set_events(eventbox, GDK_BUTTON1_MASK);
	g_signal_connect(GTK_OBJECT(eventbox), "button-press-event", G_CALLBACK(button_press), image);

	gtk_container_add(GTK_CONTAINER(eventbox), image);
	
    gtk_widget_show(eventbox);
	gtk_widget_show(image);
	gtk_widget_show(window);
	gtk_idle_add(&idle_func, image);
	gtk_main();
}

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);
	
	m.width    = std::stoi(cmd.get_parameter("width",  "640"));
	m.height   = std::stoi(cmd.get_parameter("height", "480"));
	m.center_x = std::stold(cmd.get_parameter("center-x", "-0.75"));
	m.center_y = std::stold(cmd.get_parameter("center-y", "-0.0"));
	m.scale_x  = std::stold(cmd.get_parameter("scale-x", "3.2"));
	m.scale_y  = std::stold(cmd.get_parameter("scale-y", std::to_string( (long double) m.scale_x*3.0l/4.0l).c_str()));
	update_cap = std::stoi(cmd.get_parameter("depth",  "100"));
	zoom_step  = std::stoi(cmd.get_parameter("zoom-step",  "2"));

	/*std::cout << std::setprecision(70);
	std::cout << m.width    << std::endl;
	std::cout << m.height   << std::endl;
	std::cout << m.center_x << std::endl;
	std::cout << m.center_y << std::endl;
	std::cout << m.scale_x  << std::endl;
	std::cout << m.scale_y  << std::endl;
	std::cout << update_cap << std::endl;*/

	m.generate_init();
	m.generate(update_cap);
	img = m.makeimage();
	if (cmd.has_option('s', "save"))
		img.Save("fact.bmp");

	std::cout << "done." << std::endl;

	gtk_init(&argc, &argv);
	gtk_app();
	
	//std::cout << min_f << std::endl << max_f << std::endl;
}




