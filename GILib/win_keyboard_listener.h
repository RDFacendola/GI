#pragma once

#include "timer.h"
#include "hid_listener.h"

/// The status of a keyboard's key
enum KEY_STATUS{

	UP,
	DOWN

};

/// Describes a keyboard event
struct HID_KEYBOARD_EVENT{

	float time;
	unsigned short scan_code;	///< The scan code of the keyboard's key
	KEY_STATUS key_status;

};

/// Listens to keyboard events.
class WinKeyboardListener : public HIDListener<HID_KEYBOARD_EVENT>{

public:

	WinKeyboardListener();

protected:

	/// Process an incoming message
	virtual LRESULT ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time);

};