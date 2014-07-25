#pragma once

#include <vector>
#include <assert.h>

#include "imessage_listener.h"

using ::std::vector;

///Base listener for human interface devices
template <class Event>
class HIDListener : public IMessageListener{

public:

	///Creates a new HID listener
	HIDListener(){

		event_stream_ = new vector<Event>();
		is_active_ = false;

	}

	virtual ~HIDListener(){

		delete event_stream_;

	}

	/// Activate or deactivate the listener
	__forceinline void SetActive(bool activate){

		is_active_ = activate;

	}

	/// Is the listener active?
	__forceinline bool isActive(){

		return is_active_;

	}

	/// Clear the event stream
	__forceinline void ClearEvents(){

		delete event_stream_;

		event_stream = new vector<Event>();

	}

	/// Returns the event stream and clear the current one
	__forceinline vector<Event> * GetEvents(){

		vector<Event> * old_event_stream = event_stream_;

		event_stream_ = new vector<Event>();

		return old_event_stream;

	}

	///Receive a windows message
	__forceinline LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time){

		if (isActive()){

			return ProcessMessage(window_handle, message_id, wparameter, lparameter, time);

		}

		return 0;

	}

protected:

	/// Add a new event to the stream
	__forceinline void AddEvent(const Event & event){

		event_stream_->push_back(event);

	}

	/// Process an incoming message
	virtual LRESULT ProcessMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time) = 0;

private:

	vector<Event> * event_stream_;

	bool is_active_;

};
