
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "connector.h"
#include "cmdline.h"

using namespace std::literals;

cmdline cmd;

std::string message(const std::string&);

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

gboolean idle_func(gpointer data)
{
	(void)data;
	
	if (server_instance)
	{
		server_instance->io_service.poll();
	}
	
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

extern void Save(), Load();

GtkWidget* log3[3];
std::vector<std::string> logs;
std::deque<std::string> logtop = { ""s, ""s, ""s };

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

unsigned int clients = 0;

std::string message(const std::string& msg)
{
	auto p = msg.find(":");
	std::string command = msg.substr(0, p);
	std::string detail = msg.substr(p+1);
	add_log("got command '"s + command + "' with operand '" + detail +"'");
	if (command == "send")
	{
		if (detail=="client-id")
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

void do_log()
{
	GtkWidget* subwin;
	subwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow*)subwin, "Log");
	gtk_window_set_modal((GtkWindow*)subwin, TRUE);
	gtk_window_set_default_size((GtkWindow*)subwin, 240,250);

	GtkWidget* txt = gtk_text_view_new();
	gtk_text_view_set_editable((GtkTextView*)txt, FALSE);
	GtkTextBuffer* buf = gtk_text_view_get_buffer((GtkTextView*)txt);
	std::string ss;
	for (auto&& s : logs)
		ss += s + "\n";
	gtk_text_buffer_set_text(buf, ss.c_str(), ss.length());
	gtk_container_add(GTK_CONTAINER(subwin), txt);
	gtk_widget_show_all(subwin);
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
		extern void start_server();
		start_server();
	}
	if (data == (void*)2)
	{
		add_log("Stop");
	}
	if (data == (void*)3)
	{
		add_note("Open Log");
		do_log();
	}
	if (data == (void*)4)
	{
		Save();
	}
	if (data == (void*)5)
	{
		Load();
	}
	if (data == (void*)6)
	{
		gtk_main_quit();
	}

	return TRUE;
}

void start_server()
{
    boost::asio::io_service io_service;

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
		assert( str == caps[i] );
		std::getline(ifs,str);
		gtk_entry_set_text(GTK_ENTRY(inp[i]),str.c_str());
	}
	add_log("Loaded 'Server.save'");
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
	GtkWidget* btn_save  = gtk_button_new_with_label("Save");
	GtkWidget* btn_load  = gtk_button_new_with_label("Load");
	GtkWidget* btn_exit  = gtk_button_new_with_label("Exit");

	GtkWidget* btns[] = { btn_start, btn_stop, btn_log, btn_save, btn_load, btn_exit };

	for (int i=0; i<6; ++i)
	{
		gtk_widget_show(btns[i]);
		gtk_container_add(GTK_CONTAINER(buttons), btns[i]);
		g_signal_connect(GTK_OBJECT(btns[i]), "button-press-event", G_CALLBACK(button_press), (void*)(intptr_t)(i+1));
	}

	gtk_paned_pack1(GTK_PANED (frame_main), inputbox, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED (frame_main), buttons, FALSE, FALSE);

	GtkWidget *frame_lower = gtk_vbox_new(TRUE,1);
	gtk_widget_show(frame_lower);

	for (int i=0; i<3; ++i)
	{
		log3[i] = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(frame_lower), log3[i], TRUE, TRUE, 0);
	}

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

