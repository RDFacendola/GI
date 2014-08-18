#pragma once

#include <memory>

#include "application.h"
#include "factory.h"
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
	virtual void Update(const Time & time);

private:

	Factory & factory_;

	unique_ptr<Graphics> graphics_;

};