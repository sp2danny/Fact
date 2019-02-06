
#include <iostream>
#include <iomanip>
#include <chrono>

#include <boost/filesystem.hpp>

#include "graph.h"
#include "cmdline.h"
#include "mandel_hp.h"

cmdline cmd;

Map m;
Image img;

typedef std::string Str;

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);

	static Flt   zoom_cur        = cmd.get_parameter( "zoom-start",   "3.2"     );
	static Flt   zoom_end        = cmd.get_parameter( "zoom-end",     "0"       );
	static Flt   zoom_step       = cmd.get_parameter( "zoom-step",    "0.99"    );
	static Flt   center_x        = cmd.get_parameter( "center-x",     "0.5"     );
	static Flt   center_y        = cmd.get_parameter( "center-y",     "0.0"     );
	static UL    num_dig         = std::stol  ( cmd.get_parameter ( "num-digits",   "3"       ));
	static UL    update_cap      = std::stol  ( cmd.get_parameter ( "update-cap",   "50"      ));
	static UL    image_width     = std::stol  ( cmd.get_parameter ( "width",        "640"     ));
	static UL    image_height    = std::stol  ( cmd.get_parameter ( "height",       "480"     ));
	static UL    skip_count      = std::stol  ( cmd.get_parameter ( "skip",         "0"       ));
	static UL    max_count       = std::stol  ( cmd.get_parameter ( "max-count",    "0"       ));
	static float mod_base        = std::stof  ( cmd.get_parameter ( "col-base",     "100"     ));
	static float mod_pow         = std::stof  ( cmd.get_parameter ( "col-pow",      "0.03"    ));
	static Str   target_dir      = cmd.get_parameter("target", "img");
	static Str   name_lead       = cmd.get_parameter("lead", "m_");

	auto mkname = [&](UL i) -> Str
	{
		Str name = target_dir;
		name += "/";
		name += name_lead;
		static char buff[32];
		sprintf(buff, "%0*u", (int)num_dig, i);
		name += buff;
		name += ".bmp";
		return name;
	};

	UL i = 1;

	while (skip_count)
	{
		i += 1;
		zoom_cur *= zoom_step;
		--skip_count;
	}

	Str curr_name = mkname(i);

	if (!cmd.has_option('o', "overwrite"))
	{
		while (true)
		{
			if (!boost::filesystem::exists(curr_name)) break;
			i += 1;
			zoom_cur *= zoom_step;
			curr_name = mkname(i);
		}
	}

	if (i != 1)
	{
		std::cout << "skipping 1 .. " << (i-1) << std::endl;
	}

	Map m;
	m.width  = image_width;
	m.height = image_height;
	m.center_x = center_x;
	m.center_y = center_y;
	m.zoom_mul = zoom_step;

	if (cmd.has_option('p', "print"))
	{
		std::cout << std::setprecision(75);
		std::cout << "center-x       : " << m.center_x    << std::endl;
		std::cout << "center-y       : " << m.center_y    << std::endl;
		std::cout << "update cap     : " << update_cap    << std::endl;
		std::cout << "zoom step      : " << zoom_step     << std::endl;
	}

	bool ten = cmd.has_option('t', "ten");
	bool owr = cmd.has_option('o', "overwrite");
	bool prt = cmd.has_option('p', "print");
	bool i25 = cmd.has_option('q', "25");
	
	if (ten && i25)
	{
		std::cerr << "-t and -q are exclusive" << std::endl;
		return -1;
	}

	while (true)
	{
		if (zoom_end)
		{
			if (zoom_step < 1) {
				if (zoom_cur < zoom_end) break;
			} else {
				if (zoom_cur > zoom_end) break;
			}
		}

		if (max_count)
			if (i>max_count) break;

		if (!owr && boost::filesystem::exists(curr_name))
		{
			i += 1;
			zoom_cur *= zoom_step;
			curr_name = mkname(i);
			continue;
		}

		m.scale_x = zoom_cur;
		m.scale_y = (zoom_cur * (Flt)image_height) / (Flt)image_width;

		if (prt)
		{
			std::cout << std::setprecision(25);
			std::cout << "scale-x        : " << m.scale_x << std::endl;
			std::cout << "scale-y        : " << m.scale_y << std::endl;
		}

		ModFunc mod_func = [](double d) { return mod_base / (float)pow(d, mod_pow); };

		if (ten)
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			UL maxout = m.generate_10_threaded(update_cap, mod_func);
			//UL maxout = m.generate_10(update_cap, mod_func);
			auto t2 = std::chrono::high_resolution_clock::now();
			if (prt)
			{
				std::cout << "generate time  : " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() / 1000.0f << std::endl;
				std::cout << "effective cap  : " << maxout << std::endl;
				std::cout << "color mod      : " << mod_func(zoom_cur.get_d()) << std::endl;
			}
			for (int j=0; j<10; ++j)
			{
				curr_name = mkname(i+j);
				zoom_cur *= zoom_step;
				if (!owr && boost::filesystem::exists(curr_name)) continue;
				m.makeimage_10_N(j,mod_func).Save(curr_name);
				std::cout << "Wrote: " << curr_name << std::endl;
			}
			i += 10;
		}
		else if (i25)
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			UL maxout = m.generate_25_threaded(update_cap, mod_func);
			auto t2 = std::chrono::high_resolution_clock::now();
			if (prt)
			{
				std::cout << "generate time  : " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() / 1000.0f << std::endl;
				std::cout << "effective cap  : " << maxout << std::endl;
				std::cout << "color mod      : " << mod_func(zoom_cur.get_d()) << std::endl;
			}
			for (int j=0; j<25; ++j)
			{
				curr_name = mkname(i+j);
				zoom_cur *= zoom_step;
				if (!owr && boost::filesystem::exists(curr_name)) continue;
				m.makeimage_25_N(j,mod_func).Save(curr_name);
				std::cout << "Wrote: " << curr_name << std::endl;
			}
			i += 25;
		}
		else
		{
			m.generate_init();
			m.generate(update_cap,true);
			m.makeimage( mod_func(zoom_cur.get_d()) ).Save(curr_name);
			zoom_cur *= zoom_step;
			++i;
		}
		curr_name = mkname(i);
	}

	std::cout << "\ndone.\n";
}


