#include "win_mouse_listener.h"

#include <Windows.h>

#include "exceptions.h"

WinMouseListener::WinMouseListener() : HIDListener(){

	const unsigned short kHIDUsagePageGeneric = 0x01;
	const unsigned short kHIDUsageMouse = 0x02;

	//Register the keyboard

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = kHIDUsagePageGeneric;
	Rid[0].usUsage = kHIDUsageMouse;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = 0;

	if (!RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))){

		throw RuntimeException(L"Unable to register the mouse interface");

	}

}

LRESULT WinMouseListener::ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time){

	if (message_id == WM_INPUT){

		const unsigned short kBufferLength = 40;

		unsigned int buffer_size = kBufferLength;
		unsigned char buffer[kBufferLength];
		RAWINPUT * raw_input = (RAWINPUT*)buffer;

		GetRawInputData((HRAWINPUT)lparameter, RID_INPUT, buffer, &buffer_size, sizeof(RAWINPUTHEADER));

		// Is the input message from the mouse?
		if (raw_input->header.dwType == RIM_TYPEMOUSE)
		{

			const RAWMOUSE & raw_mouse = raw_input->data.mouse;

			// These are needed only if the movement received was in absolute coordinates.
			// And I'm not sure when this could (or might) happen...
			static long last_x = 0;
			static long last_y = 0;

			// Fill the event
			HID_MOUSE_EVENT mouse_event;

			ZeroMemory(&mouse_event, sizeof(mouse_event));

			mouse_event.time = time.totalSeconds;
			mouse_event.movement_x = raw_mouse.lLastX - last_x;
			mouse_event.movement_y = raw_mouse.lLastY - last_y;

			if (raw_input->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE){

				//Corrects the data whether the coordinate received where absolute
				mouse_event.movement_x -= last_x;
				mouse_event.movement_y -= last_y;

				last_x = raw_mouse.lLastX;
				last_y = raw_mouse.lLastY;

			}

			if (raw_mouse.usButtonFlags & RI_MOUSE_WHEEL){

				mouse_event.wheel = raw_mouse.usButtonData;

			}

			mouse_event.left_button_up = (raw_mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) > 0;
			mouse_event.right_button_up = (raw_mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) > 0;
			mouse_event.middle_button_up = (raw_mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) > 0;
			mouse_event.button_4_up = (raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) > 0;
			mouse_event.button_5_up = (raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) > 0;

			mouse_event.left_button_up = (raw_mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) > 0;
			mouse_event.right_button_down = (raw_mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) > 0;
			mouse_event.middle_button_down = (raw_mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) > 0;
			mouse_event.button_4_down = (raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) > 0;
			mouse_event.button_5_down = (raw_mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) > 0;

			AddEvent(mouse_event);

		}

	}

	return 0;

}