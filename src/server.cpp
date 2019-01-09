
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

/*gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	(void)widget;
	(void)data;
	switch (event->keyval)
	{
	case GDK_Escape:
		gtk_main_quit();
		break;
	default:
		break;
	}

	return TRUE;
}*/

gboolean nil_cb(GtkWidget*, gpointer) { return TRUE; }

gboolean button_press(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	if (event->type != GDK_BUTTON_PRESS)
		return TRUE;

	if (data == (void*)1)
	{
		printf("Start\n");
	}
	if (data == (void*)2)
	{
		printf("Stop\n");
	}
	if (data == (void*)3)
	{
		printf("Log\n");
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

	GtkWidget* pane = gtk_vpaned_new();

	GtkWidget* frame_main = gtk_hpaned_new();
	gtk_widget_show(frame_main);
	
	const char* caps[] = { "center-x", "center-y", "update-cap", "zoom-start", "zoom-end", "width", "height", "col-base", "col-pow" };	
	const int N = sizeof(caps) / sizeof(char*);
	GtkWidget* inp[N];
	GtkWidget* lbl[N];

	GtkWidget* inputbox = gtk_table_new (2, N, FALSE);
	gtk_widget_show(inputbox);

	for (int i=0; i<N; ++i)
	{
		lbl[i] = gtk_label_new(caps[i]);
		gtk_widget_show(lbl[i]);
		gtk_table_attach(GTK_TABLE(inputbox), lbl[i], 0, 1, i, i+1, GTK_SHRINK, GTK_SHRINK, 2,2);
		inp[i] = gtk_entry_new();
		gtk_widget_show(inp[i]);
		gtk_table_attach(GTK_TABLE(inputbox), inp[i], 1, 2, i, i+1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), GTK_SHRINK, 2,2);
	}

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

	gtk_paned_pack1(GTK_PANED (frame_main), inputbox, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED (frame_main), buttons, FALSE, FALSE);

	GtkWidget *frame_lower = gtk_frame_new (NULL);
	gtk_widget_show(frame_lower);

	gtk_paned_pack1 (GTK_PANED (pane), frame_main, TRUE, TRUE);

	gtk_paned_pack2 (GTK_PANED (pane), frame_lower, FALSE, FALSE);
	gtk_widget_set_size_request (frame_lower, -1, 50);

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

