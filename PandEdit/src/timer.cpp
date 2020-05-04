//  ===== Date Created: 04 May, 2020 ===== 

#include "timer.hpp"

Timer::Timer(std::string&& scopeName)
	: scopeName(scopeName)
{
	reset();
}

Timer::~Timer()
{
	if (scopeName != "")
	{
		printf("Info: Timer '%s' took %fms.\n", scopeName.c_str(), getElapsedMs());
	}
}

void Timer::reset()
{
	start = Clock::now();
}

double Timer::getElapsedMs()
{
	std::chrono::duration<double, std::milli> diff = Clock::now() - start;
	return diff.count();
}
