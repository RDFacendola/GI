#include "timer.h"

using namespace gi_lib;

Time Timer::GetTime(){

	high_resolution_clock::time_point now = clock_.now();

	auto total = now - start_;
	auto delta = now - last_;

	last_ = now;

	return Time(total,delta);

}
