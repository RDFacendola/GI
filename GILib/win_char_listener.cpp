#include "win_char_listener.h"

#include <Windows.h>

#include "exceptions.h"

WinCharListener::WinCharListener() : HIDListener(){

	// Nothing the do here...

}

LRESULT WinCharListener::ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time){

	if (message_id == WM_CHAR){

		const long kMultiplicityMask = 0xF; //The number of times the character is repeated is packed into the 15 less significative bits of lparameter

		//Nice and easy

		HID_CHAR_EVENT char_event;

		char_event.time = time.totalSeconds;
		char_event.character = (char32_t)wparameter;
		char_event.multiplicity = lparameter & kMultiplicityMask;

		AddEvent(char_event);

	}

	return 0;

}