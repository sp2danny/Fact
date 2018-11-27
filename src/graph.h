
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <functional>

typedef unsigned char UC;

struct RGB
{
	UC r, g, b;
};

struct Image
{
	Image(int, int);
	void PutPixel(int, int, RGB);
	void Save(std::ostream&);
	void Save(std::string fn);
private:
	int idx(int, int) const;
	int w, h;
	std::vector<RGB> pixels;
};






