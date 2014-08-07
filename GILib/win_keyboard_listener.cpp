#include "win_keyboard_listener.h"

#include <Windows.h>

#include "exceptions.h"

WinKeyboardListener::WinKeyboardListener() : HIDListener(){

	const unsigned short kHIDUsagePageGeneric = 0x01;
	const unsigned short kHIDUsageKeyboard = 0x06;

	// Register the keyboard
	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = kHIDUsagePageGeneric;
	Rid[0].usUsage = kHIDUsageKeyboard;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = 0;

	if (!RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))){

		throw RuntimeException(L"Unable to register the keyboard interface");

	}

}

LRESULT WinKeyboardListener::ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const Timer::Time & time){

	if (message_id == WM_INPUT){

		const unsigned short kBufferLength = 40;

		unsigned int buffer_size = kBufferLength;
		unsigned char buffer[kBufferLength];
		RAWINPUT * raw_input = (RAWINPUT*)buffer;

		GetRawInputData((HRAWINPUT)lparameter, RID_INPUT, buffer, &buffer_size, sizeof(RAWINPUTHEADER));

		// Is the input message from the keyboard?
		if (raw_input->header.dwType == RIM_TYPEKEYBOARD)
		{

			const RAWKEYBOARD & raw_keyboard = raw_input->data.keyboard;

			HID_KEYBOARD_EVENT keyboard_event;;

			ZeroMemory(&keyboard_event, sizeof(keyboard_event));

			// Fill the event
			keyboard_event.time = time.GetTotalSeconds();
			keyboard_event.scan_code = raw_keyboard.MakeCode;

			if (raw_keyboard.Flags & RI_KEY_BREAK){

				keyboard_event.key_status = KEY_STATUS::UP;

			}
			else{

				keyboard_event.key_status = KEY_STATUS::DOWN;

			}

			AddEvent(keyboard_event);

		}

	}

	return 0;

}