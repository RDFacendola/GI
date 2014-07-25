#pragma once

#include "timer.h"
#include "hid_listener.h"

/// Describes a char event
struct HID_CHAR_EVENT{

	float time;
	char32_t character;
	unsigned short multiplicity;

};

/// Listens to keyboard events.
class WinCharListener : public HIDListener<HID_CHAR_EVENT>{

public:

	WinCharListener();

protected:

	/// Process an incoming message
	virtual LRESULT ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time);

};
