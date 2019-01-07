

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

#include "connector.h"

#include "cmdline.h"

cmdline cmd;


gboolean idle_func(gpointer data)
{
	(void)data;
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



gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	(void)widget;
	(void)data;
	switch (event->keyval)
	{
	case GDK_Q:
	case GDK_q:
	case GDK_Escape:
		gtk_main_quit();
		break;
	default:
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

	return TRUE;
}

void gtk_app()
{
	GtkWidget* window;
	//GtkWidget* image;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), nullptr);

	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	GtkWidget* pane = gtk_vpaned_new();
	
	GtkWidget* frame_main  = gtk_vbutton_box_new ();
	gtk_widget_show(frame_main);
	
	GtkWidget* btn_start = gtk_button_new_with_label("start");
	GtkWidget* btn_stop  = gtk_button_new_with_label("stop");

	gtk_widget_show(btn_start);
	gtk_widget_show(btn_stop);
	gtk_container_add(GTK_CONTAINER(frame_main), btn_start);
	gtk_container_add(GTK_CONTAINER(frame_main), btn_stop);

	GtkWidget *frame_lower = gtk_frame_new (NULL);
	gtk_widget_show(frame_lower);

	gtk_paned_pack1 (GTK_PANED (pane), frame_main, TRUE, TRUE);

	gtk_paned_pack2 (GTK_PANED (pane), frame_lower, FALSE, FALSE);
	gtk_widget_set_size_request (frame_lower, -1, 50);

	gtk_container_add(GTK_CONTAINER(window), pane);
	gtk_widget_show(pane);

	//GtkWidget* eventbox = gtk_event_box_new();
	//gtk_container_add(GTK_CONTAINER(window), eventbox);

	/*
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
*/

	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press), nullptr);
	//gtk_widget_set_events(eventbox, GDK_BUTTON1_MASK);
	//g_signal_connect(GTK_OBJECT(eventbox), "button-press-event", G_CALLBACK(button_press), nullptr);

	//gtk_container_add(GTK_CONTAINER(eventbox), nullptr);//image);

    //gtk_widget_show(eventbox);
	///gtk_widget_show(image);
	gtk_widget_show(window);
	gtk_idle_add(&idle_func, nullptr);
	gtk_main();
}

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);

	gtk_init(&argc, &argv);
	gtk_app();
}

