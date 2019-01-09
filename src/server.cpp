
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

	GtkWidget* inputbox = gtk_vbox_new (FALSE, 8);
	gtk_widget_show(inputbox);

	GtkWidget* inp[4];
	//GtkEntryBuffer* eb[4];
	for (int i=0; i<4; ++i)
	{

		inp[i] = gtk_entry_new ();
		//gtk_entry_set_max_length (GTK_ENTRY (inp[i]), 50);
		//gtk_editable_set_editable (GTK_EDITABLE (inp[i]), TRUE);
		//g_object_set (inp[i], "can-focus", TRUE, NULL);
		//g_signal_connect (inp[i], "activate", G_CALLBACK (enter_callback), inp[i]);
		//gtk_entry_set_text (GTK_ENTRY (inp[i]), "123");
		gtk_box_pack_start (GTK_BOX (inputbox), inp[i], TRUE, TRUE, 0);
		gtk_widget_show (inp[i]);
		
		/*
		inp[i] = gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(inp[i]), TRUE);
		//gtk_widget_set_events(inp[i], GDK_KEY_PRESS_MASK);
		//g_signal_connect (inp[i], "activate", G_CALLBACK (nil_cb), inp[i]);
		gtk_widget_set_can_focus(inp[i], TRUE);
		//eb[i] = gtk_entry_buffer_new("", 0);
		//g_signal_connect(inp[i], "key-press-event", G_CALLBACK(key_press), eb[i]);
		//gtk_entry_buffer_set_max_length(eb[i], 500);
		gtk_entry_set_max_length (GTK_ENTRY(inp[i]), 500);
		//gtk_entry_set_buffer (GTK_ENTRY(inp[i]), eb[i]);
		gtk_entry_set_editable(GTK_ENTRY(inp[i]), TRUE);
		gtk_widget_show(inp[i]);
		gtk_container_add(GTK_CONTAINER(inputbox), inp[i]);
		*/
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

	//g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press), nullptr);

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

