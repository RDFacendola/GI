#include "application.h"	

#include <string>

#ifdef _WIN32

#include <Windows.h>

#endif

#include "time.h"
#include "system.h"
#include "exceptions.h"

using namespace gi_lib;
using namespace std;

// Global stuffs

namespace{

#ifdef _WIN32

	const int kUnitLabelLength = 3;
	const wchar_t * kExtensionSeparator = L".";
	const wchar_t * kPathSeparator = L"\\";
	const wchar_t * kWindowClassName = L"GiLibWindow";

#endif

}

// Application

Application::Application(){

#ifdef _WIN32

	//The used memory should be low enough to keep these data in memory...
	RegisterWindowClass();
	
#endif

}

Application::~Application(){

	UnregisterWindowClass();

}

wstring Application::GetPath() const{

#ifdef _WIN32

	wchar_t path_buffer[MAX_PATH + 1];

	GetModuleFileName(0, path_buffer, sizeof(path_buffer));

	return wstring(path_buffer);

#else

	static_assert(false, "Unsupported OS");

#endif

}

wstring Application::GetName(bool extension) const{

	auto  path = GetPath();

	return path.substr(static_cast<unsigned int>(path.find_last_of(kExtensionSeparator)),
					   static_cast<unsigned int>(path.find_last_of(kPathSeparator)));

}

void Application::DisposeWindow(const Window::Handle & handle){

	//The shared pointer's destructor will take care of destroying the window correctly
	auto window = windows_.find(handle);

	if (window != windows_.end()){

		windows_.erase(window);

	}
	
}

weak_ptr<Window> Application::GetWindow(const Window::Handle & handle){

	auto window = windows_.find(handle);

	if (window != windows_.end()){

		return window->second;

	}else{

		return weak_ptr<Window>();

	}

}

void Application::Join(){

#ifdef _WIN32

	MSG message;

	Timer timer;

	//Loops while there are windows
	while (windows_.size() > 0){

		//If there's a message, handle it. Update the windows normally otherwise.
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) == TRUE){

			TranslateMessage(&message);
			DispatchMessage(&message);

		}

		//Shared time to enforce coherence (windows may use their own timer to have the actual time).
		for (auto & window : windows_){

			(window.second)->Update(timer.GetTime());

		}

	}

#else

	static_assert(false, "Unsupported OS");

#endif

}

#ifdef _WIN32

LRESULT __stdcall Application::ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	auto window = Application::GetInstance().GetWindow(window_handle);

	// Proper receiver.
	if (auto temp_window = window.lock()){

		return temp_window->ReceiveMessage(message_id, wparameter, lparameter);

	}

	// Default behavior when no receiver is found.
	return DefWindowProc(window_handle, message_id, wparameter, lparameter);

}

#include <map>

void Application::RegisterWindowClass(){

	auto instance = GetModuleHandle(nullptr);

	//Extract the first icon found into the executable
	window_icon_ = ExtractIcon(instance,
								Application::GetInstance().GetPath().c_str(), 0);

	WNDCLASS window_description;

	ZeroMemory(&window_description, sizeof(WNDCLASS));

	window_description.lpszClassName = kWindowClassName;
	window_description.style = CS_VREDRAW | CS_HREDRAW;
	window_description.hIcon = window_icon_;
	window_description.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_description.hInstance = instance;
	window_description.lpfnWndProc = Application::ReceiveMessage;

	SetLastError(0);

	//Attempt to register the class
	THROW_ON_ERROR(RegisterClass(&window_description));

}

void Application::UnregisterWindowClass(){

	if (window_icon_ != nullptr){

		DestroyIcon(window_icon_);

		window_icon_ = nullptr;

	}

	UnregisterClass(kWindowClassName,
					GetModuleHandle(nullptr));

}

#endif

// Application::Window

Window::Window(){

#ifdef _WIN32
		
	//Create the window
	THROW_ON_ERROR( handle_ = CreateWindowW(kWindowClassName,
											L"",
											WS_OVERLAPPEDWINDOW,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											NULL,
											NULL,
											GetModuleHandle(nullptr),
											NULL));

#else

	static_assert(false, "Unsupported OS");

#endif

}

Window::~Window(){

#ifdef _WIN32

	DestroyWindow(handle_);

#else

	static_assert(false, "Destroy the window here");

#endif

}

void Window::SetTitle(const wstring & title){

#ifdef _WIN32

	SetWindowText(handle_, title.c_str());

#else

	static_assert(false, "Unsupported OS");

#endif

}

void Window::Show(bool show){

#ifdef _WIN32

	ShowWindow(handle_, 
			   show ? SW_SHOW : SW_HIDE);


#else

	static_assert(false, "Unsupported OS");

#endif
	
}