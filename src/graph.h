
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <functional>

typedef unsigned char UC;

#pragma pack(push, 1)

struct RGB
{
	UC b, g, r;
};

#pragma pack(pop)

struct Image
{
	Image(int, int);
	void PutPixel(int, int, RGB);
	void Save(std::ostream&);
	void Save(std::string fn);
	UC* data() { return (UC*)pixels.data(); }
	int Width() const { return w; }
	int Height() const { return h; }
private:
	int idx(int, int) const;
	int w, h;
	std::vector<RGB> pixels;
};






