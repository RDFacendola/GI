#pragma once

#include <map>
#include <string>
#include <Windows.h>

#include "observable.h"
#include "imessage_listener.h"
#include "timer.h"

using ::std::map;
using ::std::wstring;

class IWindowProc;
class Window;

///
class Application{

public:

	///Add a new windowed logic to the application
	static void AddLogic(HINSTANCE application_instance, IWindowProc & logic);

	///Remove an existing window and its logic
	static void RemoveLogic(HWND & window_handle);

	///Starts the application's loop
	static void Run();

private:

	Application(){}

	///Manages the incoming windows messages
	static LRESULT __stdcall ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

	///Collections of windows inside this application
	static map<HWND, Window *> & GetWindows(){

		static map<HWND, Window *> windows;

		return windows;

	}
	
};

///Interface for the application logic
class IWindowProc{

public:

	///Initialize the logic
	virtual void Initialize(Window & window) = 0;

	///Destroy the logic
	virtual void Destroy() = 0;
	
	///Receive a message from the OS
	virtual LRESULT ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter) = 0;

	///Run a "frame" of application logic
	virtual void Update(HWND window_handle, const Timer::Time & time) = 0;

};

///Window
class Window{

public:

	typedef Observable<Window &, unsigned int, WPARAM, LPARAM> TMessageEvent;

	///Create a new window
	Window(HINSTANCE application_instance, WNDPROC dispatcher, IWindowProc & logic);

	///Destroy this window
	~Window();

	///Initialize the logic
	void Initialize();

	///Receive a message from the OS
	LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

	///Run a "frame" of application logic
	void Update(const Timer::Time & time);

	///Get the window handle
	HWND GetWindowHandle(){

		return window_handle_;

	}
	
	///Event raised whenever a message is received by the window
	Observable<Window &, unsigned int, WPARAM, LPARAM> & OnMessage(){

		return on_message_;

	}

private:

	///Create a new window
	static void CreateNew(HINSTANCE application_instance, WNDPROC dispatcher, HWND & window_handle, HICON & icon_handle);

	IWindowProc & logic_;

	HWND window_handle_;

	HICON icon_handle_;
	
	Event<Window &, unsigned int, WPARAM, LPARAM> on_message_;

};