
#include "graph.h"

#include <fstream>
#include <cstring>
#include <cassert>
#include <cmath>

#include "common.h"

Image::Image()
{
	w = h = 0;
}

Image::Image(int w, int h)
	: w(w), h(h)
{
	assert(w && h);
	pixels.resize(w*h, {255,255,255});
}

void Image::PutPixel(int x, int y, RGB pix)
{
	if (x <  0) return;
	if (x >= w) return;
	if (y <  0) return;
	if (y >= h) return;
	pixels[idx(x,y)] = pix;
}

RGB Image::GetPixel(int x, int y) const
{
	if ((x <  0) ||
	    (x >= w) ||
	    (y <  0) ||
	    (y >= h) )
		throw "Range error";
	return pixels[idx(x,y)];
}

void Image::Save(std::string fn)
{
	std::ofstream ofs(fn, std::ios_base::binary);
	Save(ofs);
}

#pragma pack( push, 1 )

struct BITMAP_FILE_HEADER
{
	unsigned char magic[2];
	int file_size;
	short reserved1;
	short reserved2;
	int image_data_offset;
};

struct BITMAP_INFO_HDR
{
	int hdr_size;
	int img_width;
	int img_height;
	short col_planes;
	short bpp;
	int compr_meth;
	int img_size;
	int hor_dpi;
	int ver_dpi;
	int col_pal_sz;
	int col_pal_important;
}; 

#pragma pack(pop)

void Image::Save(std::ostream& out)
{
	BITMAP_FILE_HEADER bfh;

	int scanlinesize = 3*w;
	int padbytes = 0;
	while( (scanlinesize%4) != 0 ) { ++scanlinesize; ++padbytes; }

	bfh.magic[0] = 'B' ;
	bfh.magic[1] = 'M' ;
	bfh.file_size = sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HDR) + scanlinesize*h;
	bfh.reserved1 = 0;
	bfh.reserved2 = 0;
	bfh.image_data_offset = sizeof(BITMAP_FILE_HEADER) + sizeof(BITMAP_INFO_HDR);

	BITMAP_INFO_HDR bih;

	std::memset( &bih, 0, sizeof(BITMAP_INFO_HDR) );

	bih.hdr_size = sizeof(BITMAP_INFO_HDR);
	bih.img_width = w;
	bih.img_height = -h;
	bih.col_planes = 1;
	bih.bpp = 24;
	bih.compr_meth = 0;
	bih.img_size = scanlinesize*h;
	bih.hor_dpi = 2835;
	bih.ver_dpi = 2835;

	out.write( (char*) & bfh , sizeof(BITMAP_FILE_HEADER) );
	out.write( (char*) & bih , sizeof(BITMAP_INFO_HDR) );

	for(int y=0;y<h;++y)
	{
		for(int x=0;x<w;++x)
		{
			RGB& pix = pixels[idx(x,y)];

			out.write( (char*) & pix.r, 1 );
			out.write( (char*) & pix.g, 1 );
			out.write( (char*) & pix.b, 1 );
		}
		for(int i=0;i<padbytes;++i)
		{
			out.write( "" , 1 );
		}
	}
}

int Image::idx(int x, int y) const
{
	return x + y*w;
}

RGB mix(const RGB& p1,const RGB& p2, float f)
{
	assert( (f >= 0.0f) && (f <= 1.0f) );
	const float inv = 1.0f-f;
	float r = p1.r * f + p2.r * inv;
	float g = p1.g * f + p2.g * inv;
	float b = p1.b * f + p2.b * inv;
	int ri = clamp((int)roundl(r), 0, 255);
	int gi = clamp((int)roundl(g), 0, 255);
	int bi = clamp((int)roundl(b), 0, 255);
	return {(UC)ri, (UC)gi, (UC)bi};
}
