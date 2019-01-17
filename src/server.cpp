
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

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "connector.h"
#include "cmdline.h"

using namespace std::literals;

cmdline cmd;

std::string message(const std::string&);

struct JobSession
{
	int frame_start;
	int frame_count;
};

std::map<int, JobSession> jobs;

struct Server
{
    boost::asio::io_service io_service;
    server serv;
	
	Server(short p)
		: io_service()
		, serv(io_service, p, &message)
	{}
};

std::unique_ptr<Server> server_instance;

extern gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data);

gboolean idle_func(gpointer data)
{
	(void)data;
	
	if (server_instance)
	{
		server_instance->io_service.poll();
	}
	
	return TRUE;
}

gboolean nil_cb(GtkWidget*, gpointer) { return TRUE; }

extern void Save(), Load();

GtkWidget* log3[3];
std::vector<std::string> logs;
std::deque<std::string> logtop = { ""s, ""s, ""s };

extern std::string add_time(std::string s);

void add_log(std::string s)
{
	logtop.push_back(s);
	logtop.pop_front();
	logs.push_back(add_time(s));
	for (int i=0; i<3; ++i)
	{
		auto str = logtop[i];
		if (i==2)
			str = ">>> " + str + " <<<";
		gtk_label_set_text(GTK_LABEL(log3[i]), str.c_str());
	}
}

const char* caps[] = {
	"center-x", "center-y", "update-cap", "zoom-start", "zoom-end",
	"width", "height", "col-base", "col-pow", "port"
};
const int N = sizeof(caps) / sizeof(char*);
GtkWidget* inp[N];
GtkWidget* lbl[N];

int clients = 0;
int frames = 0;

std::mutex lock;

std::string message(const std::string& msg)
{
	auto p = msg.find(":");
	std::string command = msg.substr(0, p);
	std::string detail = msg.substr(p+1);
	add_log("got command '"s + command + "' with operand '" + detail +"'");
	if (command == "send")
	{
		if (detail == "client-id")
		{
			++clients;
			add_log("did reply : "s + std::to_string(clients));
			return std::to_string(clients);
		}
		else
		{
			for (int i=0; i<N; ++i)
			{
				if (detail == caps[i])
				{
					const char* str = gtk_entry_get_text(GTK_ENTRY(inp[i]));
					add_log("did reply : "s + str);
					return str;
				}
			}
		}
	}
	else if (command == "request")
	{
		lock.lock();
		JobSession js{frames, 100};
		frames += 100;
		jobs[std::stoi(detail)] = js;
		lock.unlock();
		return std::to_string(js.frame_start) + "," + std::to_string(js.frame_count);
	}
	else if (command == "close")
	{
		return "OK";
	}
	return "NOK";
}

void add_note(std::string s)
{
	logs.push_back(add_time(s));
}

extern void do_log();
extern void start_server();

typedef void (*VFunc)();

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

void start_server()
{
	add_log("Start");
	const char* str = gtk_entry_get_text(GTK_ENTRY(inp[9]));
	server_instance = std::make_unique<Server>(std::atoi(str));
}

void Save()
{
	std::ofstream ofs{"Server.save"};
	for (int i=0; i<N; ++i)
	{
		ofs << caps[i] << ":";
		const char* str = gtk_entry_get_text(GTK_ENTRY(inp[i]));
		ofs << str << std::endl;
	}
	add_log("Saved 'Server.save'");
}

void Load()
{
	std::ifstream ifs{"Server.save"};
	for (int i=0; i<N; ++i)
	{
		std::string str;
		std::getline(ifs,str,':');
		assert(str == caps[i]);
		std::getline(ifs,str);
		gtk_entry_set_text(GTK_ENTRY(inp[i]),str.c_str());
	}
	add_log("Loaded 'Server.save'");
}

struct Btn
{
	GtkWidget* btn;
	std::string lbl;
	VFunc func;
};

std::vector<Btn> btn_lst = {
	Btn{nullptr, "Start" , start_server },
	Btn{nullptr, "Stop"  , [](){add_log("Stop");} },
	Btn{nullptr, "Log"   , do_log },
	Btn{nullptr, "Save"  , Save },
	Btn{nullptr, "Load"  , Load },
	Btn{nullptr, "Exit"  , gtk_main_quit }
};

void gtk_app()
{
	GtkWidget* window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), nullptr);

	gtk_container_set_border_width(GTK_CONTAINER(window), 8);

	GtkWidget* pane = gtk_vpaned_new();

	GtkWidget* frame_main = gtk_hpaned_new();
	gtk_widget_show(frame_main);

	GtkWidget* inputbox = gtk_table_new(2, N, FALSE);
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

	for (auto&& btn : btn_lst)
	{
		btn.btn = gtk_button_new_with_label(btn.lbl.c_str());
		gtk_widget_show(btn.btn);
		gtk_container_add(GTK_CONTAINER(buttons), btn.btn);
		g_signal_connect(GTK_OBJECT(btn.btn), "button-press-event", G_CALLBACK(button_press), (void*)btn.func);
	}

	gtk_paned_pack1(GTK_PANED(frame_main), inputbox, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(frame_main), buttons, FALSE, FALSE);

	GtkWidget *frame_lower = gtk_vbox_new(TRUE, 1);
	gtk_widget_show(frame_lower);

	for (int i=0; i<3; ++i)
	{
		log3[i] = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(frame_lower), log3[i], TRUE, TRUE, 0);
	}

	gtk_paned_pack1(GTK_PANED(pane), frame_main, TRUE, TRUE);

	gtk_paned_pack2(GTK_PANED(pane), frame_lower, FALSE, FALSE);
	gtk_widget_set_size_request (frame_lower, -1, 50);

	gtk_container_add(GTK_CONTAINER(window), pane);
	gtk_widget_show(pane);

	gtk_widget_show_all(window);
	gtk_widget_show(window);
	gtk_idle_add(&idle_func, nullptr);
	gtk_main();
}


