//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(TIMER_HPP)
#define TIMER_HPP

#include <chrono>
#include <string>

class Timer
{
	using Clock = std::chrono::high_resolution_clock;

private:
	Clock::time_point start;
	std::string scopeName = "";

public:
	Timer(std::string&& scopeName = "");
	~Timer();

	void reset();
	double getElapsedMs();
};

#endif
