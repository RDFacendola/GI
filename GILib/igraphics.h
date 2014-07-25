#pragma once

#include "system_profiler.h"

enum class ANTIALIASING_MODE{

	NONE,
	MSAA_2X,
	MSAA_4X,
	MSAA_8X,
	MSAA_16X,

};

struct GRAPHIC_MODE{

	unsigned int horizontal_resolution;
	unsigned int vertical_resolution;
	bool vsync;
	bool windowed;
	ANTIALIASING_MODE antialiasing;

};

class IGraphics{

public:

	///Create the graphic interface. Throws on failure
	virtual void CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode) = 0;

	///Finalize the current frame and present it to the screen
	virtual void Present() = 0;
	
};