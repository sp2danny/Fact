
#pragma once

#include <iostream>

typedef std::ostream& (&omanip_t)(std::ostream&);

struct MultiLogger
{
	void add_target(std::ostream& out)
	{
		outputs.push_back(out);
	}
	MultiLogger& operator<<(omanip_t manip)
	{
		for (auto& out : outputs)
			out.get() << manip;
		return *this;
	}
	template<typename T>
	MultiLogger& operator<<(T&& val)
	{
		std::stringstream ss;
		ss << val;
		for (auto& out : outputs)
			out.get() << ss.str();
		return *this;
	}
private:
	std::vector<std::reference_wrapper<std::ostream>> outputs;
};
