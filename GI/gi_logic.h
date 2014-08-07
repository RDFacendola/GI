#pragma once

#include <d3d11.h>

#include "application.h"
#include "igraphics.h"

///Application's logic
class GILogic : public IWindowProc{

public:

	GILogic();

	///Initialize the logic
	virtual void Initialize(Window & window);

	///Destroy the logic
	virtual void Destroy();

	///Receive a message from the OS
	virtual LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

	///Run a "frame" of application logic
	virtual void Update(HWND window_handle, const Timer::Time & time);

private:

	///API factory
	IFactory * factory_;

	///Graphic object
	BaseGraphics * graphics_;

};