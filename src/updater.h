
#pragma once

#include <mutex>

struct Updater
{
	static void Init(int);
	static void Tick();
	static void Display();
	static int Get();
private:
	static std::mutex lck;
	static int count, max, last;
};

