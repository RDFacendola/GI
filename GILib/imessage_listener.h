#pragma once

#include <Windows.h>

#include "timer.h"

///Expose methods to receive windows messages and react to them
class IMessageListener{

public:

	///Receive a windows message
	virtual LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time) = 0;

};