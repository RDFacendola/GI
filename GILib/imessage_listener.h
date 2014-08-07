#pragma once

#include <Windows.h>

#include "time.h"

///Expose methods to receive windows messages and react to them
class IMessageListener{

public:

	///Receive a windows message
	virtual LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const Timer::Time & time) = 0;

};