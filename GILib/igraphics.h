#pragma once

#include <Windows.h>

#include <vector>
#include <string>

using ::std::vector;
using ::std::wstring;

/// Describes the video mode
struct VIDEO_MODE{

	unsigned int horizontal_resolution;
	unsigned int vertical_resolution;
	unsigned int refresh_rate_Hz;

};

enum class ANTIALIASING_MODE{

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
	wstring adapter_name;
	vector<const VIDEO_MODE> supported_video;
	vector<const ANTIALIASING_MODE> supported_antialiasing;

};

class IFactory;
class BaseGraphics;
class IResources;
class IRenderer;

///Interface for graphic API
class IFactory{

public:
	
	virtual ~IFactory(){}

	///Get the capabilities for this API
	virtual ADAPTER_PROFILE GetProfile() const = 0;

	///Create the graphic object
	virtual BaseGraphics * Create(const HWND & window_handle) = 0;

};


///Interface for graphic device
class BaseGraphics{

public:

	virtual ~BaseGraphics(){}

	///Get the resources' loader
	virtual IResources & GetResources() = 0;

	///Get the renderer
	virtual IRenderer & GetRenderer() = 0;

	///Show the current frame and prepare the next one
	virtual void NextFrame() = 0;

	///Set the video mode
	inline virtual void SetVideo(const VIDEO_MODE & video){

		memcpy_s(&video_,
				 sizeof(video_),
				 &video,
				 sizeof(video));

	}

	///Get the current video mode
	inline const VIDEO_MODE & GetVideo() const{

		return video_;

	}

	///Set the antialising mode
	inline virtual void SetAntialising(const ANTIALIASING_MODE & antialiasing){

		memcpy_s(&antialiasing_,
				 sizeof(antialiasing_),
				 &antialiasing,
				 sizeof(antialiasing));

	}

	///Get the current antialiasing mode
	inline const ANTIALIASING_MODE & GetAntialiasing() const{

		return antialiasing_;

	}

	///Enable or disable the fullscreen state
	inline virtual void EnableFullscreen(bool enable){

		fullscreen_ = enable;

	}

	///Is the application running in fullscreen mode?
	inline bool IsFullscreenEnabled(){

		return fullscreen_;

	}

	///Enable or disable the VSync
	inline virtual void EnableVSync(bool enable){

		vsync_ = enable;

	}

	///Is the VSync enabled?
	inline bool IsVSyncEnabled(){

		return vsync_;

	}
	
private:

	VIDEO_MODE video_;

	ANTIALIASING_MODE antialiasing_;

	bool fullscreen_;

	bool vsync_;

};

///Used to load resources
class IResources{



};

///Used to draw the scene
class IRenderer{

};