
#include "updater.h"

#include <iostream>

// ----------------------------------------------------------------------------

// ***************l
// *** Updater ***
// ***************

std::mutex Updater::lck;
int Updater::count, Updater::max, Updater::last;

void Updater::Init(int m)
{
	count = 0;
	max = m;
	last = -1;
}

void Updater::Tick()
{
	std::lock_guard guard(lck);
	++count;
}

void Updater::Display()
{
	std::lock_guard guard(lck);
	float f = 100.0f;
	f /= max;
	f *= count;
	int p = (int)f;
	if (p==last) return;
	std::cout << p << "%\r" << std::flush;
	last = p;
}

int Updater::Get()
{
	std::lock_guard guard(lck);
	float f = 100.0f;
	f /= max;
	f *= count;
	return (int)f;
}



