#pragma once

#include "timer.h"
#include "hid_listener.h"

/// Describes a mouse event
struct HID_MOUSE_EVENT{

	float time;
	long movement_x;
	long movement_y;
	long wheel;
	bool left_button_up : 1;
	bool right_button_up : 1;
	bool middle_button_up : 1;
	bool button_4_up : 1;
	bool button_5_up : 1;
	bool left_button_down : 1;
	bool right_button_down : 1;
	bool middle_button_down : 1;
	bool button_4_down : 1;
	bool button_5_down : 1;

};

/// Listens to mouse events.
class WinMouseListener : public HIDListener<HID_MOUSE_EVENT>{

public:

	WinMouseListener();

protected:

	/// Process an incoming message
	virtual LRESULT ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time);

};