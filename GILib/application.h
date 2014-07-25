#pragma once

#include <string>
#include <Windows.h>

#include "imessage_listener.h"
#include "timer.h"

using ::std::wstring;

class IApplicationLogic;

///Manages a windows application
class Application{

public:

	static Application & GetSingleton();

	///Starts the application's main loop
	/** The method call BaseApplicationLogic::ReceiveMessage when a new windows message is received and BaseApplicationLogic::Update when no
	message is available. */
	void Run(HINSTANCE application_instance, IApplicationLogic & application_logic);

private:

	static unsigned int kApplicationLogicMessage;	///< Used to send the pointer to the application's logic

	static unsigned int kApplicationTimeMessage;	///< Used to send the pointer to the application's time

	Application(){}

	/// Manages the incoming windows messages
	/** \param message_id The id of the message catched */
	static LRESULT __stdcall ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

	/// Create a new window
	/** \param application_instance The application instance
	\param title The application title */
	HWND CreateApplicationWindow(HINSTANCE application_instance, const IApplicationLogic & application_logic);

	/// Clean the application up
	void CleanUp(HWND window_handle, HICON icon_handle, IApplicationLogic & application_logic);

};

///Interface for an application's logic
class IApplicationLogic : public IMessageListener{

public:

	///Initialize the application logic
	virtual void Initialize(HWND window_handle) = 0;

	///Run a "frame" of application logic
	virtual void Update(HWND window_handle, const APPLICATION_TIME & time) = 0;

	///Destroy the application logic
	virtual void Destroy() = 0;

	///Is the logic still running?
	virtual bool IsRunning() const = 0;

	///Declares that the logic is no longer running
	virtual void Exit() = 0;

	///Get the window's width in pixel
	virtual unsigned int GetWindowWidth() const = 0;

	///Get the window's height in pixel
	virtual unsigned int GetWindowHeight() const = 0;

	///Get the window's title
	virtual wstring GetWindowTitle() const = 0;

};