
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "connector.h"
#include "cmdline.h"

using namespace std::literals;

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
std::deque<std::string> logtop(13);

std::string add_time(std::string s)
{
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T > ") << s;
    return ss.str();
}

void add_log(std::string s)
{
	logtop.push_back(s);
	logtop.pop_front();

	logs.push_back(add_time(s));

	for (int i=0; i<13; ++i)
	{
		auto str = logtop[i];
		if (i==12)
			str = ">>> "s + str.c_str() + " <<<"s;
		gtk_label_set_text(GTK_LABEL(log13[i]), str.c_str());
	}
}

void add_note(std::string s)
{
	logs.push_back(add_time(s));
}

[[maybe_unused]] constexpr int max_length = 1024;

struct Item
{
	Item(std::string name)
		: name(name), value()
	{}
	std::string name;
	std::string value;	
};

std::vector<Item> params = {
	"client-id"s, "center-x"s, "center-y"s, "update-cap"s, "zoom-start"s,
	"width"s, "height"s, "col-base"s, "col-pow"s
};

struct Connection
{
	Connection(std::string addr, std::string port)
		: io_service()
		, resolver(io_service)
		, query(tcp::v4(), addr, port)
		, iterator(resolver.resolve(query))
		, sock(io_service)
	{
		boost::asio::connect(sock, iterator);
	}

	boost::asio::io_service io_service;
    tcp::resolver resolver;
    tcp::resolver::query query;
    tcp::resolver::iterator iterator;
    tcp::socket sock;
};

std::unique_ptr<Connection> connection;

GtkWidget* window;
GtkWidget* edit;

void client_start()
{
	std::string ed = gtk_entry_get_text(GTK_ENTRY(edit));
	auto p = ed.find(':');
	std::string addr = ed.substr(0,p);
	std::string port = ed.substr(p+1);
	
	connection = std::make_unique<Connection>(addr,port);
	
	auto snd_rcv = [&](std::string msg) -> std::string
	{
		add_log("Sending : "s + msg);
		
		boost::asio::write(connection->sock, boost::asio::buffer(msg.c_str(), msg.size()));

		char reply[max_length];
		[[maybe_unused]] size_t reply_length;
		reply_length = boost::asio::read(connection->sock, boost::asio::buffer(reply, max_length));

		std::string ret((char*)reply);
		add_log("Got answer : "s + ret);
		return ret;
	};

	std::string repl;
	
	for (auto& itm : params)
	{
		itm.value = snd_rcv("send:"s + itm.name);
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
		client_start();
	}
	if (data == (void*)2)
	{
		add_log("Stop");
	}
	if (data == (void*)3)
	{
		extern void do_log();
		do_log();
	}
	if (data == (void*)4)
	{
		gtk_main_quit();
	}

	return TRUE;
}

void do_log()
{
	GtkWidget* subwin;
	subwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow*)subwin, "Log");
	gtk_window_set_modal((GtkWindow*)subwin, TRUE);

	GtkWidget* txt = gtk_text_view_new();
	GtkTextBuffer* buf = gtk_text_view_get_buffer((GtkTextView*)txt);
	std::string ss;
	for (auto&& s : logs)
		ss += s + "\n";
	gtk_text_buffer_set_text(buf, ss.c_str(), ss.length());
	gtk_container_add(GTK_CONTAINER(subwin), txt);
	gtk_widget_show_all(subwin);
}

void gtk_app()
{
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
	
	edit = gtk_entry_new();
	gtk_widget_show(edit);
	gtk_entry_set_text(GTK_ENTRY(edit),"127.0.0.1:9090");
	gtk_box_pack_start(GTK_BOX(frame_log), edit, TRUE, TRUE, 0);

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

