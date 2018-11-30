
#include "graph.h"
#include "cmdline.h"
#include "mandel.h"

#include "boost/filesystem.hpp"

cmdline cmd;

Map m;
Image img;

typedef std::string Str;

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);

	Flt   zoom_cur        = std::stold ( cmd.get_parameter( "zoom-start",   "3.2"     ));
	Flt   zoom_end        = std::stold ( cmd.get_parameter( "zoom-end",     "0.32"    ));
	Flt   zoom_step       = std::stold ( cmd.get_parameter( "zoom-step",    "0.99"    ));
	UL    num_dig         = std::stol  ( cmd.get_parameter( "num-digits",   "3"       ));
	Flt   center_x        = std::stold ( cmd.get_parameter( "center-x",     "+0.5"    ));
	Flt   center_y        = std::stold ( cmd.get_parameter( "center-y",     "0.0"     ));
	UL    update_cap      = std::stol  ( cmd.get_parameter( "update-cap",   "50"      ));
	UL    image_width     = std::stol  ( cmd.get_parameter( "width",        "640"     ));
	UL    image_height    = std::stol  ( cmd.get_parameter( "height",       "480"     ));
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

	while (true)
	{
		if (zoom_step < 1) {
			if (zoom_cur < zoom_end) break;
		} else {
			if (zoom_cur > zoom_end) break;
		}
		
		m.scale_x = zoom_cur;
		m.scale_y = (zoom_cur * (Flt)image_height) / (Flt)image_width;
		m.generate_init();
		m.generate(update_cap);
		float mod = 215 / pow(zoom_cur, 0.05);
		m.makeimage(mod).Save(curr_name);
		std::cout << "Wrote: " << curr_name << "                \r" << std::flush;
		curr_name = mkname(++i);
		zoom_cur *= zoom_step;
	}

	std::cout << "\ndone.\n";
}


