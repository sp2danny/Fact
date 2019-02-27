
#include "fixed.hpp"

#include <cmath>


/*
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
*/

static unsigned char NORMAL = 0;
static unsigned char POS    = 0;
static unsigned char NEG    = 1;
static unsigned char OVERFL = 2;
static unsigned char UNDERF = 4;

fixed3::fixed3()
{
	ip = 0;
	for (auto&& f : fp) f = 0;
	mask = NORMAL;
}

fixed3::fixed3(long lval)
{
	for (auto&& f : fp) f = 0;
	if (lval<0)
	{
		seti(-lval);
		mask = NEG;
	} else {
		seti(lval);
		mask = POS;
	}
}

void fixed3::seti(unsigned short i)
{
	ip = i;
}

fixed3::fixed3(double dval)
{
	if (dval < 0)
	{
		mask = NEG;
		dval = -dval;
	} else {
		mask = POS;
	}
	auto fl = floorl(dval);
	seti(fl);
	for (auto&& f : fp)
	{
		dval -= fl;
		dval *= 65536.0;
		fl = floorl(dval);
		f = (unsigned short)fl;
	}
}

fixed3& fixed3::operator += (const fixed3& other)
{
	//if (
}

fixed3& fixed3::operator -= (const fixed3& other)
{
}

fixed3& fixed3::operator *= (const fixed3& other)
{
}






