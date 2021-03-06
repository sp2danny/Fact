
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

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "cmdline.h"
#include "mandel_hp.h"
//#include "makemovie.h"

using boost::asio::ip::tcp;
using namespace std::literals;

cmdline cmd;

gboolean idle_func(gpointer data);

extern gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data);

gboolean nil_cb(GtkWidget*, gpointer) { return TRUE; }

GtkWidget* log13[13];
std::vector<std::string> logs;
std::deque<std::string> logtop(13);

extern std::string add_time(std::string s);

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

auto snd_rcv(std::string msg) -> std::string
{
	add_log("Sending : "s + msg);

	boost::asio::write(connection->sock, boost::asio::buffer(msg.c_str(), msg.size()));

	char reply[max_length];
	[[maybe_unused]] size_t reply_length;
	reply_length = boost::asio::read(connection->sock, boost::asio::buffer(reply, max_length));

	std::string ret((char*)reply);
	add_log("Got answer : "s + ret);
	return ret;
}

bool have_job = false;
int job_start, job_len, job_curr;

#define DV std::declval

struct pick_3          {};
struct pick_2 : pick_3 {};
struct pick_1 : pick_2 {};

template<typename T>
auto get_param(pick_2, std::string name)
	-> decltype( DV<std::istream&>() >> DV<T&>() , T{} )
{
	for (auto&& x : params)
	{
		if (name == x.name)
		{
			T ret;
			std::stringstream ss;
			ss.str(x.value.c_str());
			ss >> ret;
			return ret;
		}
	}
	return {};
}

template<typename T>
auto get_param(pick_1, std::string name)
	-> decltype( T{std::string{}}, T{} )
{
	for (auto&& x : params)
	{
		if (name == x.name)
		{
			T ret{x.value.c_str()};
			return ret;
		}
	}
	return {};
}

template<typename T>
auto get_param(pick_3, std::string name) -> T
{
	for (auto&& x : params)
	{
		if (name == x.name)
		{
			return (T)x.value.c_str();
		}
	}
	return {};
}

template<typename T>
auto get_param(std::string name) -> auto
{
	return get_param<T>(pick_1{}, name);
}

void client_start()
{
	add_log("Start");
	std::string ed = gtk_entry_get_text(GTK_ENTRY(edit));
	auto p = ed.find(':');
	std::string addr = ed.substr(0,p);
	std::string port = ed.substr(p+1);

	connection = std::make_unique<Connection>(addr,port);

	for (auto& itm : params)
	{
		itm.value = snd_rcv("send:"s + itm.name);
	}
}

auto mkname(int i) -> std::string
{
	std::string name = "./temp/img_";
	static char buff[32];
	sprintf(buff, "%05d", i);
	name += buff;
	name += ".bmp";
	return name;
};

void make_10(int f)
{
	Map<FltH> m;
	m.width    = get_param<int>("width"s);
	m.height   = get_param<int>("height"s);
	m.center_x = get_param<FltH>("center-x"s);
	m.center_y = get_param<FltH>("center-y"s);
	m.zoom_mul = 0.99;

	static double mod_base = get_param<double>("col-base"s);
	static double mod_pow  = get_param<double>("col-pow"s);

	FltH z = get_param<FltH>("zoom-start"s);
	for (int i=0; i<f; ++i)
		z *= m.zoom_mul;

	m.scale_x = z;
	m.scale_y = (z * (FltH)m.height) / (FltH)m.width;

	ModFunc mod_func = [](double d) -> float { return mod_base / (float)pow(d, mod_pow); };

	UL update_cap = get_param<UL>("update-cap"s);
	UL maxout = m.generate_N_threaded(10, update_cap, true);

	for (int j=0; j<10; ++j)
	{
		auto curr_name = mkname(f+j);
		auto img = m.makeimage_N(j, mod_func);
		img.Save(curr_name);
		//mm::AddFrame(img);
	}

	add_log("wrote 10 images");

	(void)maxout;
}

gboolean idle_func([[maybe_unused]] gpointer data)
{
	if (!connection) return TRUE;

	if (!have_job)
	{
		auto ret = snd_rcv("request:"s + params[0].value);
		auto p = ret.find(',');
		job_start = std::stoi(ret.substr(0, p));
		job_len = std::stoi(ret.substr(p+1));
		add_log("Starting job : "s + ret);
		job_curr = job_start;
		have_job = true;
		
		[[maybe_unused]] int w = get_param<int>("width");
		[[maybe_unused]] int h = get_param<int>("height");
		//mm::SetupMovie(w,h, "./temp/mov_"s + std::to_string(job_curr) + ".mp4" );
		
		return TRUE;
	}

	if (!have_job) return TRUE;

	if (job_curr < (job_start+job_len))
	{
		make_10(job_curr);
		job_curr += 10;
		return TRUE;
	}

	//mm::Encode();
	add_log("finished one batch");
	have_job = false;

	return TRUE;
}

extern void do_log();

typedef void (*VFunc)();

struct Btn
{
	GtkWidget* btn;
	std::string lbl;
	VFunc func;
};

std::vector<Btn> btn_lst = {
	Btn{nullptr, "Start" , client_start },
	Btn{nullptr, "Stop"  , [](){add_log("Stop");} },
	Btn{nullptr, "Log"   , do_log },
	Btn{nullptr, "Exit"  , gtk_main_quit }
};

gboolean button_press(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;

	if (event->type != GDK_BUTTON_PRESS)
		return TRUE;

	VFunc func = (VFunc)data;
	func();

	return TRUE;
}

void gtk_app()
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), nullptr);

	gtk_container_set_border_width(GTK_CONTAINER(window), 8);

	GtkWidget* pane = gtk_hpaned_new();
	GtkWidget* buttons = gtk_vbutton_box_new();
	gtk_widget_show(buttons);

	for (auto&& btn : btn_lst)
	{
		btn.btn = gtk_button_new_with_label(btn.lbl.c_str());
		gtk_widget_show(btn.btn);
		gtk_container_add(GTK_CONTAINER(buttons), btn.btn);
		g_signal_connect(GTK_OBJECT(btn.btn), "button-press-event", G_CALLBACK(button_press), (void*)btn.func);
	}

	GtkWidget *frame_log = gtk_vbox_new(TRUE, 1);
	gtk_widget_show(frame_log);

	edit = gtk_entry_new();
	gtk_widget_show(edit);
	gtk_entry_set_text(GTK_ENTRY(edit), "127.0.0.1:9090");
	gtk_box_pack_start(GTK_BOX(frame_log), edit, TRUE, TRUE, 0);

	for (int i=0; i<13; ++i)
	{
		log13[i] = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(frame_log), log13[i], TRUE, TRUE, 0);
	}

	gtk_paned_pack1(GTK_PANED (pane), frame_log, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED (pane), buttons, FALSE, FALSE);

	gtk_container_add(GTK_CONTAINER(window), pane);
	gtk_widget_show(pane);

	gtk_widget_show_all(window);
	gtk_widget_show(window);
	gtk_idle_add(&idle_func, nullptr);
	gtk_main();
}


