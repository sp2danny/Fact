
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <ios>

#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "graph.h"
#include "cmdline.h"
#include "mandel_hp.h"

cmdline cmd;

typedef Map<FltH> MapH;
typedef Point<FltH> PointH;

MapH m;
Image img;

unsigned long iter_init = 600;
unsigned long fuc = 1200;
unsigned long update_cap = 200;
unsigned long update_step = 20;
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

	auto res = m.generate(update_cap, true, true);
	if (res == MapH::all_done)
	{
		std::cout << " --- all done ---               " << std::endl << std::flush;
		m.map_all_done = true;
		return TRUE;
	}

	if (res == MapH::was_updated )
	{
		std::cout << "        working            " << "     \r" << std::flush;
		noupdatefor = 0;
	}
	if (res == MapH::no_change)
	{
		noupdatefor += 1;
		std::cout << "        unupdated " << noupdatefor << "     \r" << std::flush;
	}

	float mod = mod_base / (float)std::pow((double)m.scale_x, mod_pow);
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
	float mod = mod_base / (float)pow((double)m.scale_x, mod_pow);

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
	//m.generate_odd(iter_init);
	//m.generate(update_cap, true);
	std::cout << "        quad pix           " << "     \r" << std::flush;
	disp_img(image);
}

UL escape_min, escape_max, non_escape;
void find_escape()
{
	non_escape = 0;
	bool found = false;
	UL x,y;
	for (y=0; y<m.height; ++y) for (x=0; x<m.width; ++x)
	{
		auto& p = m.get(x,y);
		if (p.status == PointH::calc)
		{
			if (p.iter > non_escape) non_escape = p.iter;
			continue;
		}
		if (p.status != PointH::out) continue;
		if (!found)
		{
			escape_min = escape_max = p.iter;
			found = true;
		} else {
			if (p.iter < escape_min) escape_min = p.iter;
			if (p.iter > escape_max) escape_max = p.iter;
		}
	}
}

void kross(GtkImage* image)
{
	UL w = img.Width();
	UL h = img.Height();
	UL w12 = w / 2;
	UL w13 = w / 3;
	UL w23 = 2 * w13;
	UL h12 = h / 2;
	UL h13 = h / 3;
	UL h23 = 2 * h13;
	UL x,y;
	for (x=0; x<w13; ++x)
		img.PutPixel(x, h12, {0,0,0});
	for (x=w23; x<w; ++x)
		img.PutPixel(x, h12, {0,0,0});
		
	for (y=0; y<h13; ++y)
		img.PutPixel(w12, y, {0,0,0});
	for (y=h23; y<h; ++y)
		img.PutPixel(w12, y, {0,0,0});
	
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

/*
std::string make_string(const FltH& flt)
{
	std::string str;
	constexpr std::size_t sz = 1024;
	char buff[sz];
	FILE* fp = fmemopen(buff, sz, "w");
	auto w = mpf_out_str(fp, 10, 120, flt.get_mpf_t());
	buff[w] = 0;
	str.assign(buff, w);
	fclose(fp);
	return str;
}
*/

gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	(void)widget;
	(void)data;
	//mp_exp_t expo;
	switch (event->keyval)
	{
	case GDK_r:
		update_cap = 200;
		update_step = 20;
		mk_img((GtkImage*)data);
		break;	
	case GDK_S:
		img.Save("fact.bmp");
		break;
	case GDK_Q:
		gtk_main_quit();
		break;
	case GDK_k:
		kross((GtkImage*)data);
		break;
	case GDK_z:
		zoom_step += 1.0l;
		std::cout << "zoom step    : " << zoom_step   << std::endl;
		break;
	case GDK_x:
		zoom_step -= 1.0l;
		std::cout << "zoom step    : " << zoom_step   << std::endl;
		break;
	case GDK_space:
		update_cap = 200;
		update_step = 20;
		m.scale_x /= zoom_step;
		m.scale_y /= zoom_step;
		std::cout << "        new zoom           " << "     \r" << std::flush;
		mk_img((GtkImage*)data);
		break;
	case GDK_f:
		m.map_all_done = true;
		std::cout << "        fake done          " << "     \r" << std::flush;
		break;
	case GDK_p:
		find_escape();
		std::cout << std::setprecision(120) << std::endl;
		std::cout << "center-x     : " << m.center_x  << " (" << m.center_x .get_prec() << ") " << std::endl;
		std::cout << "center-y     : " << m.center_y  << " (" << m.center_y .get_prec() << ") " << std::endl;
		std::cout << "scale-x      : " << m.scale_x   << " (" << m.scale_x  .get_prec() << ") " << std::endl;
		std::cout << "scale-y      : " << m.scale_y   << " (" << m.scale_y  .get_prec() << ") " << std::endl;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "update step  : " << update_step << std::endl;
		std::cout << "trigger cap  : " << fuc         << std::endl;
		std::cout << "escape min   : " << escape_min  << std::endl;
		std::cout << "escape max   : " << escape_max  << std::endl;
		std::cout << "non escape   : " << non_escape  << std::endl;
		std::cout << "zoom step    : " << zoom_step   << std::endl;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow((double)m.scale_x, mod_pow) << std::endl;
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
	//case GDK_m:
	//	movie_maker(m, update_cap);
	//	break;
	case GDK_q:
		mod_base *= 0.99f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow((double)m.scale_x, mod_pow) << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_w:
		mod_base *= 1.01f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow((double)m.scale_x, mod_pow) << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_a:
		mod_pow *= 0.99f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow((double)m.scale_x, mod_pow) << std::endl;
		disp_img((GtkImage*)data);
		break;
	case GDK_s:
		mod_pow *= 1.01f;
		std::cout << "col-base     : " << mod_base    << std::endl;
		std::cout << "col-pow      : " << mod_pow     << std::endl;
		std::cout << "col-calc     : " << mod_base / (float)pow((double)m.scale_x, mod_pow) << std::endl;
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
		std::cout << "        new zoom           " << "     \r" << std::flush;
    }

    if (event->button == 1)
	{
		FltH ldx = m.to_xpos(event->x);
		FltH ldy = m.to_ypos(event->y);

		m.center_x = ldx;
		m.center_y = ldy;
		m.scale_x /= zoom_step;
		m.scale_y /= zoom_step;
		std::cout << "        new zoom & center  " << "     \r" << std::flush;
	}

	update_cap = 200;
	update_step = 20;

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

	m.width    = std::stol(  cmd.get_parameter ("width",      "640"   ));
	m.height   = std::stol(  cmd.get_parameter ("height",     "480"   ));
	m.center_x =             cmd.get_parameter ("center-x",   "-0.75" ) ;
	m.center_y =             cmd.get_parameter ("center-y",   "-0.0"  ) ;
	m.scale_x  =             cmd.get_parameter ("scale-x",    "3.2"   ) ;
	m.scale_y  = m.scale_x * double(m.height)/double(m.width);
	fuc        = std::stol(  cmd.get_parameter ("update-cap", "2500"  ));
	zoom_step  = std::stof(  cmd.get_parameter ("zoom-step",  "3.16"  ));
	mod_base   = std::stof(  cmd.get_parameter ("col-base",   "120"   ));
	mod_pow    = std::stof(  cmd.get_parameter ("col-pow",    "-0.01" ));
	iter_init  = std::stol(  cmd.get_parameter ("iter-init",  "600"   ));

	m.generate_init();
	//m.generate_odd(iter_init);
	float mod = mod_base / (float)powf((float)(double)m.scale_x, mod_pow);
	img = m.makeimage(mod, fuc);

	gtk_init(&argc, &argv);
	gtk_app();
}

