#include <exception>
#include <string>

#include "application.h"
#include "services.h"
#include "exceptions.h"
#include "raii.h"

using ::std::wstring;

///Application

///Add a new windowed logic to the application
void Application::AddLogic(HINSTANCE application_instance, IWindowProc & logic){

	auto window = new Window(application_instance, ReceiveMessage, logic);

	GetWindows()[window->GetWindowHandle()] = window;

	window->Initialize();

}

///Remove an existing logic from the application
void Application::RemoveLogic(HWND & window_handle){

	auto & windows = GetWindows();

	auto window = windows.find(window_handle);

	if (window != windows.end()){

		///Delete the window
		delete window->second;

		windows.erase(window);

	}

}

void Application::Run(){

	MSG message;
	
	Timer timer;
	float time;
	APPLICATION_TIME application_time;

	application_time.totalSeconds = 0;
			
	//Loops while there are windows
	while (GetWindows().size() > 0){

		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) == TRUE){

			//Handle the OS message
			TranslateMessage(&message);
			DispatchMessage(&message);

		}

		//Update the application time
		time = timer.GetTime();

		application_time.deltaSeconds = time - application_time.totalSeconds;
		application_time.totalSeconds = time;

		//Update every window
		for (auto & window : GetWindows()){

			(window.second)->Update(application_time);

		}

	}

}

LRESULT __stdcall Application::ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	auto & windows = GetWindows();

	auto window_iterator = windows.find(window_handle);

	if (window_iterator != windows.end()){

		if (message_id == WM_CLOSE){

			//Destroy the window
			Application::RemoveLogic(window_handle);

		}
		else{
		
			//Dispatch the message to the proper window
			return window_iterator->second->ReceiveMessage(message_id, wparameter, lparameter);

		}
		
	}

	//Default behavior
	return DefWindowProc(window_handle, message_id, wparameter, lparameter);
		
}

//Window

///Create a new window
Window::Window(HINSTANCE application_instance, WNDPROC dispatcher, IWindowProc & logic) :
	logic_(logic){

	CreateNew(application_instance, dispatcher, window_handle_, icon_handle_);

}

void Window::Initialize(){

	logic_.Initialize(*this);

}

///Destroy this window
Window::~Window(){

	DestroyWindow(window_handle_);
	DestroyIcon(icon_handle_);

	logic_.Destroy();

}

///Receive a message from the OS
LRESULT Window::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	//Raise the vent
	on_message_.Notify(*this, message_id, wparameter, lparameter);

	//Call the logic
	return logic_.ReceiveMessage(window_handle_, message_id, wparameter, lparameter);

}

///Run a "frame" of application logic
void Window::Update(const APPLICATION_TIME & time){

	logic_.Update(window_handle_, time);

}

///Create a new window
void Window::CreateNew(HINSTANCE application_instance, WNDPROC dispatcher, HWND & window_handle, HICON & icon_handle){

	icon_handle = ExtractIcon(application_instance, Services::GetApplicationPath().c_str(), 0);	//Extract the first icon found into the executable

	wstring application_name = Services::GetApplicationName();

	//Register the window class
	WNDCLASS window_description;

	ZeroMemory(&window_description, sizeof(WNDCLASS));

	window_description.lpszClassName = application_name.c_str();
	window_description.style = CS_VREDRAW | CS_HREDRAW;
	window_description.hIcon = icon_handle;
	window_description.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_description.hInstance = application_instance;
	window_description.lpfnWndProc = dispatcher;	//Callback for windows messages

	//Attempt to register the class
	if (!RegisterClass(&window_description)){

		throw RuntimeException(L"Unable to register the window class");

	}

	//Create the window
	window_handle = CreateWindow(application_name.c_str(),
								 L"",
								 WS_OVERLAPPEDWINDOW,
								 CW_USEDEFAULT,
								 CW_USEDEFAULT,
								 CW_USEDEFAULT,
								 CW_USEDEFAULT,
								 NULL,
								 NULL,
								 application_instance,
								 NULL);

	if (!window_handle){

		throw RuntimeException(L"Unable to create the window");

	}

}