
#include <iostream>
#include <iomanip>

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

	Flt   zoom_cur        = cmd.get_parameter( "zoom-start",   "3.2"     );
	Flt   zoom_end        = cmd.get_parameter( "zoom-end",     "0.32"    );
	Flt   zoom_step       = cmd.get_parameter( "zoom-step",    "0.99"    );
	Flt   center_x        = cmd.get_parameter( "center-x",     "0.5"     );
	Flt   center_y        = cmd.get_parameter( "center-y",     "0.0"     );
	UL    num_dig         = std::stol  ( cmd.get_parameter ( "num-digits",   "3"       ));
	UL    update_cap      = std::stol  ( cmd.get_parameter ( "update-cap",   "50"      ));
	UL    image_width     = std::stol  ( cmd.get_parameter ( "width",        "640"     ));
	UL    image_height    = std::stol  ( cmd.get_parameter ( "height",       "480"     ));
	UL    skip_count      = std::stol  ( cmd.get_parameter ( "skip",         "0"       ));
	UL    max_count       = std::stol  ( cmd.get_parameter ( "max-count",    "0"       ));
	float mod_base        = std::stof  ( cmd.get_parameter ( "col-base",     "100"     ));
	float mod_pow         = std::stof  ( cmd.get_parameter ( "col-pow",      "0.03"    ));
	Str   target_dir      = cmd.get_parameter("target", "img");
	Str   name_lead       = cmd.get_parameter("lead", "m_");

	auto mkname = [&](UL i) -> Str
	{
		Str name = target_dir;
		name += "/";
		name += name_lead;
		static char buff[32];
		sprintf(buff, "%0*lu", (int)num_dig, i);
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

	while (true)
	{
		if (!boost::filesystem::exists(curr_name)) break;
		i += 1;
		zoom_cur *= zoom_step;
		curr_name = mkname(i);
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
	
	if (cmd.has_option('p', "print"))
	{
		std::cout << std::setprecision(25) << std::defaultfloat;
		std::cout << "center-x     : " << m.center_x  << std::endl;
		std::cout << "center-y     : " << m.center_y  << std::endl;
		std::cout << "update cap   : " << update_cap  << std::endl;
		std::cout << "zoom step    : " << zoom_step   << std::endl;		
	}

	while (true)
	{
		if (zoom_step < 1) {
			if (zoom_cur < zoom_end) break;
		} else {
			if (zoom_cur > zoom_end) break;
		}
		
		if (max_count)
			if (i>max_count) break;

		if (boost::filesystem::exists(curr_name))
		{
			i += 1;
			zoom_cur *= zoom_step;
			curr_name = mkname(i);
			continue;
		}

		m.scale_x = zoom_cur;
		m.scale_y = (zoom_cur * (Flt)image_height) / (Flt)image_width;

		if (cmd.has_option('p', "print"))
		{
			std::cout << std::setprecision(25) << std::defaultfloat;
			std::cout << "scale-x      : " << m.scale_x << std::endl;
			std::cout << "scale-y      : " << m.scale_y << std::endl;
		}

		m.generate_init();
		m.generate(update_cap, true);
		float mod = mod_base / (float)pow(zoom_cur.get_d(), mod_pow);
		m.makeimage(mod).Save(curr_name);
		
		if (cmd.has_option('p', "print"))
		{
			std::cout << "mod          : " << mod << std::endl;
			std::cout << "Wrote: " << curr_name << std::endl;
		} else {
			std::cout << "Wrote: " << curr_name << "  zoom: " << std::setprecision(20) << zoom_cur << "                \r" << std::flush;
		}

		curr_name = mkname(++i);
		zoom_cur *= zoom_step;
	}

	std::cout << "\ndone.\n";
}


