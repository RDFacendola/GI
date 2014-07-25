#pragma once

#include <Windows.h>
#include <string>

#include "imessage_listener.h"
#include "timer.h"

using ::std::wstring;

///Interface for an application's logic
class IApplicationLogic : public IMessageListener{

public:

	///Initialize the application logic
	virtual void Initialize(HWND window_handle) = 0;

	///Run a "frame" of application logic
	virtual void Update(HWND window_handle, const APPLICATION_TIME & time) = 0;

	///Destroy the application logic
	virtual void Destroy() = 0;

	///Is the logic still running?
	virtual bool IsRunning() const = 0;

	///Declares that the logic is no longer running
	virtual void Exit() = 0;

	///Get the window's width in pixel
	virtual unsigned int GetWindowWidth() const = 0;

	///Get the window's height in pixel
	virtual unsigned int GetWindowHeight() const = 0;

	///Get the window's title
	virtual wstring GetWindowTitle() const = 0;

};
