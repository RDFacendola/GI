#pragma once

#include <memory>

#include "application.h"

#include "graphics.h"

using namespace gi_lib;
using namespace std;

///Application's logic
class GILogic : public Window{

public:

	GILogic();

	virtual ~GILogic();

	/// \brief Update the window logic.
	/// \param time The application-coherent time.
	virtual void Update(const Timer::Time & time);

private:

	IFactory & factory_;

	shared_ptr<IGraphics> graphics_;

};