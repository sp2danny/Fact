
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <fstream>

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

gboolean nil_cb(GtkWidget*, gpointer) { return TRUE; }

GtkWidget* log13[13];
std::vector<std::string> logs;

void add_log(std::string s)
{
	logs.push_back(std::move(s));
	int j, i, n = logs.size();
	i = n-13;
	if (i<0) i=0;
	for (j=0; i<n; ++i)
	{
		auto str = logs[i];
		if ((i+1)==n)
			str = ">>> " + str + " <<<";
		gtk_label_set_text(GTK_LABEL(log13[j++]), str.c_str());
	}
}

gboolean button_press(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	if (event->type != GDK_BUTTON_PRESS)
		return TRUE;

	if (data == (void*)1)
	{
		add_log("Start");
	}
	if (data == (void*)2)
	{
		add_log("Stop");
	}
	if (data == (void*)3)
	{
		add_log("Log");
	}
	if (data == (void*)4)
	{
		gtk_main_quit();
	}

	return TRUE;
}


void gtk_app()
{
	GtkWidget* window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), nullptr);

	gtk_container_set_border_width(GTK_CONTAINER(window), 8);

	GtkWidget* pane = gtk_hpaned_new();

	GtkWidget* buttons = gtk_vbutton_box_new();
	gtk_widget_show(buttons);

	GtkWidget* btn_start = gtk_button_new_with_label("Start");
	GtkWidget* btn_stop  = gtk_button_new_with_label("Stop");
	GtkWidget* btn_log   = gtk_button_new_with_label("Log");
	GtkWidget* btn_exit  = gtk_button_new_with_label("Exit");

	GtkWidget* btns[] = {btn_start, btn_stop, btn_log, btn_exit };

	for (int i=0; i<4; ++i)
	{
		gtk_widget_show(btns[i]);
		gtk_container_add(GTK_CONTAINER(buttons), btns[i]);
		g_signal_connect(GTK_OBJECT(btns[i]), "button-press-event", G_CALLBACK(button_press), (void*)(intptr_t)(i+1));
	}

	GtkWidget *frame_log = gtk_vbox_new(TRUE,1);
	gtk_widget_show(frame_log);

	for (int i=0; i<13; ++i)
	{
		log13[i] = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(frame_log), log13[i], TRUE, TRUE, 0);
	}

	gtk_paned_pack1 (GTK_PANED (pane), frame_log, TRUE, TRUE);
	gtk_paned_pack2 (GTK_PANED (pane), buttons, FALSE, FALSE);

	gtk_container_add(GTK_CONTAINER(window), pane);
	gtk_widget_show(pane);

	gtk_widget_show_all(window);
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

