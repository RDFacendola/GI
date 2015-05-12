#pragma once

#include <memory>

#include "core.h"
#include "graphics.h"
#include "scene.h"

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

	Graphics & graphics_;

	unique_ptr<IOutput> output_;

	Scene scene_;

};