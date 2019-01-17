
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
#include <deque>
#include <chrono>
#include <ctime>
#include <sstream> 
#include <memory>
#include <map>
#include <string>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

std::string add_time(std::string s)
{
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T > ") << s;
    return ss.str();
}

gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	gtk_main_quit();
	return TRUE;
}

extern std::vector<std::string> logs;

void do_log()
{
	GtkWidget* subwin;
	subwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow*)subwin, "Log");
	gtk_window_set_modal((GtkWindow*)subwin, TRUE);
	gtk_window_set_default_size((GtkWindow*)subwin, 350,450);

	GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
	GtkWidget* txt = gtk_text_view_new();
	GtkTextBuffer* buf = gtk_text_view_get_buffer((GtkTextView*)txt);

	std::string ss;
	for (auto&& s : logs)
		ss += s + "\n";
	gtk_text_buffer_set_text(buf, ss.c_str(), ss.length());
	gtk_container_add(GTK_CONTAINER(subwin), scroll);
	gtk_container_add(GTK_CONTAINER(scroll), txt);
	gtk_widget_show_all(subwin);
}

#include "cmdline.h"

extern cmdline cmd;

extern void gtk_app();

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);

	gtk_init(&argc, &argv);
	gtk_app();
}

