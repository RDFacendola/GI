#pragma once

#include <ctime>
#include <ratio>
#include <chrono>

using ::std::chrono::high_resolution_clock;
using ::std::chrono::duration;
using ::std::chrono::duration_cast;

struct APPLICATION_TIME{

	float totalSeconds;
	float deltaSeconds;

};

/**
* High-resolution time wrapper
*
* \author Raffaele D. Facendola
*/
class Timer
{

public:

	/**
	* Timer constructor. Count as a restart
	*/
	Timer(){

		Restart();

	}

	/**
	* Restart the timer
	*/
	inline void Restart(){

		start_ = clock_.now();

	}

	/**
	* Get the elapsed time since the last restart in milliseconds
	*
	* \return The elapsed time since the last restart in milliseconds
	*/
	inline float GetTime() const{

		high_resolution_clock::time_point now = clock_.now();

		duration<float> time_span = duration_cast<duration<float>>(now - start_);

		return time_span.count();

	}

private:

	high_resolution_clock clock_;

	/**
	* The instant of the last restart
	*/
	high_resolution_clock::time_point start_;

};