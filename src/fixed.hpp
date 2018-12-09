
#pragma once

#include <string>
#include <iostream>

struct fixed3
{
	fixed3();
	fixed3(long);
	fixed3(double);
	fixed3(const std::string&);

	fixed3& operator += (const fixed3&);
	fixed3& operator -= (const fixed3&);
	fixed3& operator *= (const fixed3&);
	
	fixed3& sqr();
	fixed3& dbl();
	
	std::string to_string() const;

private:

	void seti(unsigned short);

	unsigned short ip;
	unsigned short fp[3];
	unsigned char mask;
};

fixed3 operator + (const fixed3& lhs, const fixed3& rhs) { fixed3 res = lhs; res += rhs; return res; }
fixed3 operator - (const fixed3& lhs, const fixed3& rhs) { fixed3 res = lhs; res -= rhs; return res; }
fixed3 operator * (const fixed3& lhs, const fixed3& rhs) { fixed3 res = lhs; res *= rhs; return res; }

std::ostream& operator << (std::ostream& out, const fixed3& val) { out << val.to_string(); return out; }



