#pragma once

#include <d3d11.h>

#include "application.h"
#include "igraphics.h"

///Application's logic
class GILogic : public IApplicationLogic{

public:

	GILogic();

	///Initialize the application logic
	virtual void Initialize(HWND window_handle);

	///Run a "frame" of application logic
	virtual void Update(HWND window_handle, const APPLICATION_TIME & time);

	///Receive a windows message
	virtual LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time);

	///Destroy the application logic
	virtual void Destroy();

	///Is the logic still running?
	virtual bool IsRunning() const;

	///Declares that the logic is no longer running
	virtual void Exit();

	///Get the window's width in pixel
	virtual unsigned int GetWindowWidth() const;

	///Get the window's height in pixel
	virtual unsigned int GetWindowHeight() const;

	///Get the window's title
	virtual wstring GetWindowTitle() const;

private:

	///Is the application running?
	bool is_running_;

	///API factory
	IFactory * factory_;

	///Graphic object
	BaseGraphics * graphics_;

};