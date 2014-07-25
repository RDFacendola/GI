#pragma once

#include <set>
#include <vector>
#include <string>

using ::std::vector;
using ::std::set;
using ::std::wstring;
using ::std::wstringstream;

/// Describes the video mode
struct VIDEO_MODE{

	unsigned int horizontal_resolution;
	unsigned int vertical_resolution;
	unsigned int refresh_rate_Hz;

};

enum class ANTIALIASING_MODE{

	UNKNOWN,
	NONE,
	MSAA_2X,
	MSAA_4X,
	MSAA_8X,
	MSAA_16X,

};

/// Describes a video card capabilities and parameters
struct ADAPTER_PROFILE{

	size_t dedicated_memory;
	size_t shared_memory;
	wstring model_name;
	vector<const VIDEO_MODE> supported_video_modes;
	vector<const ANTIALIASING_MODE> supported_antialiasing;

};

struct GRAPHIC_MODE{

	VIDEO_MODE video;
	ANTIALIASING_MODE antialiasing;
	bool vsync;
	bool windowed;
	
};

class IGraphics{

public:

	///Get the default adapter's capabilities for this API
	virtual ADAPTER_PROFILE GetAdapterProfile() const = 0;

	///Create the graphic interface. Throws on failure
	virtual void CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode) = 0;

	///Finalize the current frame and present it to the screen
	virtual void Present() = 0;
	
};