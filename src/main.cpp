
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <ios>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "graph.h"
#include "cmdline.h"
#include "mandel.h"

cmdline cmd;

Map m;
Image img;

unsigned long fuc = 1200;
unsigned long update_cap = 100;
unsigned long update_step = 10;
float zoom_step = 2;
float mod_base = 215.f;
float mod_pow = 0.05f;
unsigned long noupdatefor = 0;

gboolean idle_func(gpointer data)
{
	if (m.map_all_done) return TRUE;

	if (noupdatefor >= 100) return TRUE;

	update_step += (update_step/10);
	update_cap += update_step;

	auto res = m.generate(update_cap);
	if (res == Map::all_done)
	{
		std::cout << " --- all done ---   " << std::endl << std::flush;
		m.map_all_done = true;
		return TRUE;
	}

	if (res == Map::was_updated)
	{
		std::cout << " working            " << "\r" << std::flush;
		noupdatefor = 0;
	}
	if (res == Map::no_change)
	{
		noupdatefor += 1;
		std::cout << " unupdated " << noupdatefor << "\r" << std::flush;
	}

	float mod = mod_base / (float)pow(m.scale_x, mod_pow);
	img = m.makeimage(mod, fuc);

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

	return TRUE;
}

gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	gtk_main_quit();
	return TRUE;
}


void disp_img(GtkImage* image)
{
	float mod = mod_base / (float)pow(m.scale_x, mod_pow);

	img = m.makeimage(mod, fuc);

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

void mk_img(GtkImage* image)
{
	noupdatefor = 0;
	m.generate_init();
	m.generate(update_cap);
	disp_img(image);
}

gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	(void)widget;
	(void)data;
	switch (event->keyval)
	{
	case GDK_r:
		update_cap = 100;
		update_step = 10;
		mk_img((GtkImage*)data);
		break;	
	case GDK_S:
		img.Save("fact.bmp");
		break;
	case GDK_Q:
		gtk_main_quit();
		break;
	case GDK_z:
		zoom_step += 1.0l;
		break;
	case GDK_x:
		zoom_step -= 1.0l;
		break;
	case GDK_space:
		update_cap = 100;
		update_step = 10;
		m.scale_x /= zoom_step;
		m.scale_y /= zoom_step;
		mk_img((GtkImage*)data);
		break;
	case GDK_p:
		std::cout << std::setprecision(22) << std::defaultfloat;
		std::cout << "center-x     : " << m.center_x  << std::endl;
		std::cout << "center-y     : " << m.center_y  << std::endl;
		std::cout << "scale-x      : " << m.scale_x   << std::endl;
		std::cout << "scale-y      : " << m.scale_y   << std::endl;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "update step  : " << update_step << std::endl;
		std::cout << "trigger cap  : " << fuc         << std::endl;
		std::cout << "zoom step    : " << zoom_step   << std::endl;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow(m.scale_x, mod_pow) << std::endl;
		break;
	case GDK_c:
		fuc *= 1.1;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "update step  : " << update_step << std::endl;
		std::cout << "trigger cap  : " << fuc         << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_v:
		fuc *= 0.9;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "update step  : " << update_step << std::endl;
		std::cout << "trigger cap  : " << fuc         << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_m:
		movie_maker(m, update_cap);
		break;
	case GDK_q:
		mod_base *= 0.99f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_w:
		mod_base *= 1.01f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_a:
		mod_pow *= 0.99f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_s:
		mod_pow *= 1.01f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		disp_img((GtkImage*)data);
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
		Flt ldx = m.to_xpos(event->x);
		Flt ldy = m.to_ypos(event->y);

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
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), nullptr);

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

	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press), image);
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

	m.width    = std::stoi(  cmd.get_parameter ("width",      "640"   ));
	m.height   = std::stoi(  cmd.get_parameter ("height",     "480"   ));
	m.center_x = std::stold( cmd.get_parameter ("center-x",   "-0.75" ));
	m.center_y = std::stold( cmd.get_parameter ("center-y",   "-0.0"  ));
	m.scale_x  = std::stold( cmd.get_parameter ("scale-x",    "3.2"   ));
	m.scale_y  = std::stold( cmd.get_parameter ("scale-y",    std::to_string((Flt)m.scale_x*3.0l/4.0l).c_str()));
	fuc        = std::stoi(  cmd.get_parameter ("update-cap", "2500"  ));
	zoom_step  = std::stoi(  cmd.get_parameter ("zoom-step",  "3.16"  ));
	mod_base   = std::stof(  cmd.get_parameter ("col-base",   "120"   ));
	mod_pow    = std::stof(  cmd.get_parameter ("col-pow",    "-0.01" ));

	m.generate_init();
	float mod = mod_base / (float)pow(m.scale_x, mod_pow);
	img = m.makeimage(mod, fuc);
	//img = m.makeimage(225);
	//if (cmd.has_option('s', "save"))
	//	img.Save("fact.bmp");

	//std::cout << "done." << std::endl;

	gtk_init(&argc, &argv);
	gtk_app();
}

