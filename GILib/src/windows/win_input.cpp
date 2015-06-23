#ifdef _WIN32

#include "exceptions.h"

#include "windows\win_input.h"

using namespace gi_lib;
using namespace gi_lib::windows;

namespace{

	/// \brief To use along with the RegisterRawInputDevices function.
	const unsigned short kHIDUsagePageGeneric = 0x01;

	/// \brief Identifies a mouse. To use along with the RegisterRawInputDevices function.
	const unsigned short kHIDUsageMouse = 0x02;

	/// \brief Identifies a keyboard. To use along with the RegisterRawInputDevices function.
	const unsigned short kHIDUsageKeyboard = 0x06;

	/// \brief Get the current input data size.
	/// The method query for the size of the current input packet.
	/// \return Return the current input data size in bytes.
	unsigned int GetInputDataSize(LPARAM lparameter){

		unsigned int input_data_size;
		
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparameter), 
						RID_INPUT, 
						nullptr, 
						&input_data_size,
						sizeof(RAWINPUTHEADER));

		return input_data_size;

	}

	/// \brief Read the current input data.
	/// \param buffer Buffer containing the raw input data. Output parameter.
	/// \return Returns true if data were read, returns false otherwise.
	bool ReadInputData(LPARAM lparameter, vector<char>& buffer){

		auto size = GetInputDataSize(lparameter);

		buffer.resize(size);

		return GetRawInputData(reinterpret_cast<HRAWINPUT>(lparameter),
							   RID_INPUT,
							   &buffer[0],
							   &size,
							   sizeof(RAWINPUTHEADER)) == size;

	}

}

/////////////////////////// INPUT //////////////////////////////

bool Input::ReceiveMessage(unsigned int message_id, WPARAM, LPARAM lparameter, LRESULT& result){

	if (message_id == WM_INPUT){

		std::vector<char> input_buffer;
		
		if (ReadInputData(lparameter, input_buffer)){

			// Find the correct type input and dispatch to the proper peripheral.

			RAWINPUT& raw_input = *reinterpret_cast<RAWINPUT*>(&input_buffer[0]);

			if (raw_input.header.dwType == RIM_TYPEMOUSE){

				mouse_.UpdateStatus(raw_input.data.mouse);

				result = 0;

				return true;

			}
			else if (raw_input.header.dwType == RIM_TYPEKEYBOARD){

				keyboard_.UpdateStatus(raw_input.data.keyboard);

				result = 0;

				return true;

			}

		}
			
	}

	return false;

}

/////////////////////////// MOUSE //////////////////////////////

Mouse::Mouse(){

	//Mouse registration

	RAWINPUTDEVICE input_device[1];

	input_device[0].usUsagePage = ::kHIDUsagePageGeneric;
	input_device[0].usUsage = ::kHIDUsageMouse;
	input_device[0].dwFlags = 0;
	input_device[0].hwndTarget = 0;

	if (!RegisterRawInputDevices(input_device, 
								 1, 
								 sizeof(input_device[0]))){

		THROW(L"Unable to register the mouse handler");

	}

}

void Mouse::UpdateStatus(const RAWMOUSE& mouse_status){

	// Buttons 

	static unsigned short down_flags[] = { RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_5_DOWN };
	static unsigned short up_flags[] = { RI_MOUSE_BUTTON_1_UP, RI_MOUSE_BUTTON_2_UP, RI_MOUSE_BUTTON_3_UP, RI_MOUSE_BUTTON_4_UP, RI_MOUSE_BUTTON_5_UP };

	for (unsigned short button_index = 0; button_index < sizeof(down_flags) / sizeof(unsigned short); ++button_index){

		if (mouse_status.usButtonFlags & down_flags[button_index]){

			down_buttons_.insert(button_index);

			pressed_buttons_.insert(button_index);

		}
		
		if (mouse_status.usButtonFlags & up_flags[button_index]){

			down_buttons_.erase(button_index);

			released_buttons_.insert(button_index);

		}
		
	}

	// Wheel

	if (mouse_status.usButtonFlags & RI_MOUSE_WHEEL){


		wheel_delta_ += reinterpret_cast<const short&>(mouse_status.usButtonData);

	}

	// Cursor movement

	if (!(mouse_status.usFlags & MOUSE_MOVE_ABSOLUTE)){

		movement_ += Vector2i(mouse_status.lLastX,
							  mouse_status.lLastY);

	}
	
	// Cursor position

	POINT position;

	if (GetCursorPos(&position)){

		position_ = Vector2i(position.x,
							 position.y);

	}

}

void Mouse::Flush(){

	// Pressed and released button states are temporary.

	released_buttons_.clear();
	pressed_buttons_.clear();

	// Zero-out every delta

	wheel_delta_ = 0.0f;
	movement_ = Vector2i::Zero();
	
}

/////////////////////////// KEYBOARD //////////////////////////////

Keyboard::Keyboard(){

	// Keyboard registration

	RAWINPUTDEVICE input_device[1];

	input_device[0].usUsagePage = ::kHIDUsagePageGeneric;
	input_device[0].usUsage = ::kHIDUsageKeyboard;
	input_device[0].dwFlags = 0;
	input_device[0].hwndTarget = 0;

	if (!RegisterRawInputDevices(input_device, 
								 1, 
								 sizeof(input_device[0]))){

		THROW(L"Unable to register the keyboard handler");

	}

}

void Keyboard::UpdateStatus(const RAWKEYBOARD& keyboard_status){

	auto key_code = keyboard_status.MakeCode;

	if (keyboard_status.Flags & RI_KEY_BREAK){

		// The key was released
		down_keys_.erase(key_code);

		released_keys_.insert(key_code);

	}
	else{

		// The key was pressed
		down_keys_.insert(key_code);

		pressed_keys_.insert(key_code);

	}
	
}

void Keyboard::Flush(){

	// Pressed and released key states are temporary.

	released_keys_.clear();
	pressed_keys_.clear();

}

#endif