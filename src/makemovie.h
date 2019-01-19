
#pragma once

#include <vector>

#include "graph.h"

namespace mm
{
	
typedef std::vector<unsigned char> DataT;

void SetupMovie(int,int);
void AddFrame(const Image&);
void Encode();
DataT GetData();

}

