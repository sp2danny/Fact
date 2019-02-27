
#include "cmdline.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using namespace std::literals;

void cmdline::init(int argc, char* argv[])
{
	assert(argc>=1);
	program_name = argv[0];
	for(auto i=1; i<argc; ++i)
	{
		if (argv[i] == "-f"s)
		{
			++i;
			if (i<argc)
			{
				std::ifstream ifs(argv[i]);
				std::string str;
				while (ifs >> str)
				{
					arguments.push_back(str);
				}
			}
		} else {
			arguments.push_back(argv[i]);
		}
	}
}

cmdline::cmdline(int argc, char* argv[])
{
	init(argc, argv);
}

bool cmdline::has_option(char ch, const char* longname, bool default_on)
{
	bool res = default_on;
	for (auto&& s : arguments)
	{
		if (s.length() < 2) continue;
		if (s[0] == '-')
		{
			if (s[1] == '-')
			{
				if (longname)
				{
					if (s.substr(2) == longname)
						res = true;
				}
			} else {
				if (s.substr(1).find_first_of(ch) != std::string::npos)
					res = true;
			}
		}
		else if (s[0] == '+')
		{
			if (s.substr(1).find_first_of(ch) != std::string::npos)
				res = false;
		}
	}
	return res;
}

std::string cmdline::get_parameter(const char* name, const char* default_value)
{
	std::string res = default_value;
	for (auto&& s : arguments)
	{
		auto p = s.find_first_of('=');
		if (p == std::string::npos)
			continue;
		if (s.substr(0,p) == name)
		{
			res = s.substr(p+1);
		}
	}
	return res;
}

std::vector<std::string> cmdline::get_commands()
{
	std::vector<std::string> res;
	for (auto&& s : arguments)
	{
		if (s.empty()) continue;
		if (s[0] == '-') continue;
		if (s[0] == '+') continue;
		if (s.find_first_of('=') == std::string::npos) continue;
		res.push_back(s);
	}
	return res;
}

