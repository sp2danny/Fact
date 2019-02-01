
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
#include <stdexcept>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

/*
namespace std
{
	template<typename _CharT>
	struct _Put_time
	{
		const std::tm* _M_tmb;
		const _CharT* _M_fmt;
    };

	template<typename _CharT>
    inline _Put_time<_CharT>
    put_time(const std::tm* __tmb, const _CharT* __fmt)
    { return { __tmb, __fmt }; }
	template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os, _Put_time<_CharT> __f)
    {
		typename basic_ostream<_CharT, _Traits>::sentry __cerb(__os);
		if (__cerb)
        {
			ios_base::iostate __err = ios_base::goodbit;
			try
            {
				typedef ostreambuf_iterator<_CharT, _Traits>   _Iter;
				typedef time_put<_CharT, _Iter>                _TimePut;
				const _CharT* const __fmt_end = __f._M_fmt +
					_Traits::length(__f._M_fmt);
				const _TimePut& __mp = use_facet<_TimePut>(__os.getloc());
				if (__mp.put(_Iter(__os.rdbuf()), __os, __os.fill(),
					__f._M_tmb, __f._M_fmt, __fmt_end).failed())
				__err |= ios_base::badbit;
			}
			catch(__cxxabiv1::__forced_unwind&)
			{
				__os._M_setstate(ios_base::badbit);
				__throw_exception_again;
			}
			catch(...)
			{ __os._M_setstate(ios_base::badbit); }
			if (__err)
				__os.setstate(__err);
		}
		return __os;
	}
}
*/

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
	try
	{
		cmd.init(argc, argv);
		gtk_init(&argc, &argv);
		gtk_app();
	}
	catch (const std::exception& msg)
	{
		std::cerr << "Error: " << msg.what() << std::endl;
	}
	catch (const std::string& msg)
	{
		std::cerr << "Error: " << msg << std::endl;
	}
	catch (const char* msg)
	{
		std::cerr << "Error: " << msg << std::endl;
	}
	catch (int msg)
	{
		std::cerr << "Error: #" << msg << std::endl;
	}
	catch (...)
	{
		std::cerr << "Error: <Unknown>" << std::endl;
	}
}

