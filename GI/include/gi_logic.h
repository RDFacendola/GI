#pragma once

#include "application.h"

using ::gi_lib::Window;
using ::gi_lib::Timer;

class Timer;

///Application's logic
class GILogic : public Window{

public:

	GILogic();

	virtual ~GILogic();

	/// \brief Update the window logic.
	/// \param time The application-coherent time.
	virtual void Update(const Timer::Time & time);

private:

	unsigned long counter;

};