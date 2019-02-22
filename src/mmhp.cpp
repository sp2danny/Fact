
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <functional>
#include <sstream>

#include <boost/filesystem.hpp>

#include "graph.h"
#include "cmdline.h"
#include "mandel_hp.h"
#include "multilogger.hpp"

using namespace std::literals;

namespace fs = boost::filesystem;

cmdline cmd;

typedef std::string Str;

int main(int argc, char* argv[])
{
	cmd.init(argc, argv);

	static FltH    zoom_cur        = from_stringH ( cmd.get_parameter ( "zoom-start",   "3.2"     ));
	static FltH    zoom_end        = from_stringH ( cmd.get_parameter ( "zoom-end",     "0"       ));
	static FltH    zoom_step       = from_stringH ( cmd.get_parameter ( "zoom-step",    "0.99"    ));
	static FltH    center_x        = from_stringH ( cmd.get_parameter ( "center-x",     "0.5"     ));
	static FltH    center_y        = from_stringH ( cmd.get_parameter ( "center-y",     "0.0"     ));
	static float   mod_base        = std::stof    ( cmd.get_parameter ( "col-base",     "100"     ));
	static float   mod_pow         = std::stof    ( cmd.get_parameter ( "col-pow",      "0.03"    ));
	static UL      num_dig         = std::stol    ( cmd.get_parameter ( "num-digits",   "3"       ));
	static UL      update_cap      = std::stol    ( cmd.get_parameter ( "update-cap",   "50"      ));
	static UL      image_width     = std::stol    ( cmd.get_parameter ( "width",        "640"     ));
	static UL      image_height    = std::stol    ( cmd.get_parameter ( "height",       "480"     ));
	static UL      skip_count      = std::stol    ( cmd.get_parameter ( "skip",         "0"       ));
	static UL      max_count       = std::stol    ( cmd.get_parameter ( "max-count",    "0"       ));
	static Str     target_dir      =                cmd.get_parameter ( "target",       "img"     ) ;
	static Str     name_lead       =                cmd.get_parameter ( "lead",         "m_"      ) ;

	auto mkname = [](UL i) -> Str
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

	UL i = 0;

	while (skip_count)
	{
		i += 1;
		zoom_cur *= zoom_step;
		--skip_count;
	}

	Str curr_name = mkname(i);
	
	bool owr = cmd.has_option('o', "overwrite");
	bool prt = cmd.has_option('p', "print");
	bool ten = cmd.has_option('t', "ten");
	bool i25 = cmd.has_option('q', "25");
	bool i50 = cmd.has_option('f', "50");
	bool dbl = cmd.has_option('d', "dbl");
	bool rep = cmd.has_option('r', "rep");
	bool mbg = cmd.has_option('m', "mbg");

	OSP fr = nullptr;
	std::ofstream fra;
	if (rep)
	{
		fra.open("FrameReport.txt"s, std::ios_base::out | std::ios_base::trunc);
		fr = &fra;
	}

	MultiLogger logger;

	if (prt)
		logger.add_target(std::ref(std::cout));
	if (rep)
		logger.add_target(std::ref(*fr));

	if (!owr)
	{
		while (true)
		{
			if (!fs::exists(curr_name)) break;
			i += 1;
			zoom_cur *= zoom_step;
			curr_name = mkname(i);
		}
	} else {
		std::cout << "running in owerwrite-mode" << std::endl;
	}

	if (i != 0)
	{
		std::cout << "skipping 0 .. " << (i-1) << std::endl;
	}

	Map<FltH> mh;
	mh.width  = image_width;
	mh.height = image_height;
	mh.center_x = center_x;
	mh.center_y = center_y;
	mh.zoom_mul = zoom_step;
	
	Map<FltL> ml;
	ml.width  = image_width;
	ml.height = image_height;
	ml.center_x = (double)center_x;
	ml.center_y = (double)center_y;
	ml.zoom_mul = (double)zoom_step;

	logger << std::setprecision(75);
	logger << "center-x       : " << mh.center_x   << std::endl;
	logger << "center-y       : " << mh.center_y   << std::endl;
	logger << "update cap     : " << update_cap    << std::endl;
	logger << "zoom step      : " << zoom_step     << std::endl;

	if ( (ten+i25+i50+dbl) > 1 )
	{
		std::cerr << "-t, -q, -f and -d are exclusive" << std::endl;
		return -1;
	}

	bool first = true;
	bool useh = false;

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

		if ((!owr) && boost::filesystem::exists(curr_name))
		{
			i += 1;
			zoom_cur *= zoom_step;
			curr_name = mkname(i);
			if (dbl)
				first = true;
			continue;
		}

		{
			bool new_useh = zoom_cur < 1e-10;
			if (useh != new_useh)
				first=true;
			useh = new_useh;
		}

		logger << "using "s << (useh ? "high"s : "low"s) << std::endl;

		mh.setZ((FltH)zoom_cur);
		ml.setZ((FltL)zoom_cur);

		if (useh) Point<FltH>::stepsize = mh.to_xpos(1) - mh.to_xpos(0);
		else      Point<FltL>::stepsize = ml.to_xpos(1) - ml.to_xpos(0);

		logger << std::setprecision(25);
		logger << "scale-x        : " << mh.scale_x << std::endl;
		logger << "scale-y        : " << mh.scale_y << std::endl;

		ModFunc mod_func = [](float d) -> float { return mod_base / powf(d, mod_pow); };
		UL maxout;

		auto EXEC = [&](int n) -> void
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			if (useh) maxout = mh.generate_N_threaded(n, update_cap, prt);
			else      maxout = ml.generate_N_threaded(n, update_cap, prt);
			auto t2 = std::chrono::high_resolution_clock::now();
			auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
			logger << "generate time  : " << d1 / 1000.0f << std::endl;
			logger << "effective cap  : " << maxout << std::endl;
			logger << "color mod      : " << mod_func((float)(double)zoom_cur) << std::endl;
			for (int j=0; j<n; ++j)
			{
				curr_name = mkname(i+j);
				zoom_cur *= zoom_step;
				if (owr && boost::filesystem::exists(curr_name)) continue;
				logger << i+j << "\r" << std::flush;
				if (useh) mh.makeimage_N(j,mod_func).Save(curr_name);
				else      ml.makeimage_N(j,mod_func).Save(curr_name);
			}
			auto t3 = std::chrono::high_resolution_clock::now();
			auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3-t1).count();
			logger << "effectiveness  : " << 1000.0f * n / d2 << std::endl;
			logger << "Wrote: " << mkname(i) << " to " << mkname(i+n-1) << std::endl;
			auto d3 = std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count();
			if (mbg) logger << "bitmap generation time : " << (d3 / n) << std::endl;
			i += n;
		};

		if (ten)
		{
			EXEC(10);
		}
		else if (i25)
		{
			EXEC(25);
		}
		else if (i50)
		{
			EXEC(50);
		}
		
		#undef EXEC
		
		else if (dbl)
		{
			int start = 0;
			if (i)
			{
				start = 1;
				i -= 1;
				zoom_cur /= zoom_step;
				mh.setZ((FltH)zoom_cur);
				ml.setZ((FltL)zoom_cur);
			}

			//std::cout << zoom_cur << std::endl;
			auto t1 = std::chrono::high_resolution_clock::now();
			int cpy = 0;
			if (first)
			{
				if (useh) mh.setup_dbl((FltH)zoom_step, logger);
				else      ml.setup_dbl((FltL)zoom_step, logger);
			} else {
				if (useh) cpy = mh.shuffle_dbl();
				else      cpy = ml.shuffle_dbl();
			}
			if (cpy) {
				logger << "moved pixels   : " << cpy << std::endl;
			}
			int n;
			if (useh) n = mh.generate_dbl(update_cap, first, prt, logger);
			else      n = ml.generate_dbl(update_cap, first, prt, logger);
			auto t2 = std::chrono::high_resolution_clock::now();
			auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
			logger << "generate time  : " << d1 / 1000.0f << std::endl;
			logger << "color mod      : " << mod_func((float)(double)zoom_cur) << std::endl;
			auto zc = zoom_cur;

			/* */
			for (int j=start; j<n; ++j)
			{
				curr_name = mkname(i+j);
				if ((!owr) && boost::filesystem::exists(curr_name)) continue;
				if (prt) std::cout << i+j << "\r" << std::flush;
				if (rep) (*fr) << "Frame " << i+j << " : ";
				if (useh) mh.makeimage_N(j,mod_func, fr).Save(curr_name);
				else      ml.makeimage_N(j,mod_func, fr).Save(curr_name);
				if (rep) (*fr) << std::endl;
			}
			/* */

			//if (useh) mh.makeimage_ItoN(start, i+start, n-start, mod_func, mkname, fr);
			//else      ml.makeimage_ItoN(start, i+start, n-start, mod_func, mkname, fr);

			auto t3 = std::chrono::high_resolution_clock::now();
			auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3-t1).count();
			if (prt)
			{
				std::cout << "effectiveness  : " << 1000.0f * n / d2 << std::endl;
			}
			std::cout << "Wrote: " << mkname(i+1) << " to " << mkname(i+n-1) << std::endl;
			auto d3 = std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count();
			if (mbg) logger << "bitmap generation time : " << (d3 / (n-start)) << " ms/image " << std::endl;
			i += n;
			zoom_cur *= FltH(0.5);
			first = false;
		}
		else
		{
			if (useh)
			{
				mh.generate_init();
				mh.generate(update_cap, true);
				mh.makeimage(mod_func((float)(double)zoom_cur)).Save(curr_name);
			} else {
				ml.generate_init();
				ml.generate(update_cap, true);
				ml.makeimage(mod_func((float)(double)zoom_cur)).Save(curr_name);
			}
			zoom_cur *= zoom_step;
			++i;
		}
		curr_name = mkname(i);
	}

	std::cout << "\ndone.\n";
}


