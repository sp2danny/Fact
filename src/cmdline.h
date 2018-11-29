
#pragma once

#include <string>
#include <vector>

class cmdline
{
public:
	cmdline() = default;
	cmdline(int argc, char* argv[]);
	void init(int argc, char* argv[]);

	bool has_option(char ch, const char* longname = nullptr, bool default_on = false);

	std::string get_parameter(const char* name, const char* default_value = "");

	std::vector<std::string> get_commands();

private:
	std::string program_name;
	std::vector<std::string> arguments;
};

