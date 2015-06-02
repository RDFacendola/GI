#include "..\include\core.h"

#include "..\include\exceptions.h"
#include "..\include\timer.h"

#include "..\include\windows\win_core.h"

using namespace gi_lib;
using namespace std;

//////////////////////////////////// SYSTEM /////////////////////////////////////////

System& System::GetInstance(){

#ifdef _WIN32

	static windows::System system;

#endif

	return system;
	
}

////////////////////// FILESYSTEM ////////////////////////////////////////

FileSystem& FileSystem::GetInstance(){

#ifdef _WIN32

	static windows::FileSystem file_system;

#endif

	return file_system;

}

////////////////////// APPLICATION ///////////////////////////////

Application& Application::GetInstance(){

#ifdef _WIN32

	static windows::Application application;

#endif

	return application;

}

void Application::DisposeWindow(const WindowHandle& handle){

	//The shared pointer's destructor will take care of destroying the window correctly
	auto window = windows_.find(handle);

	if (window != windows_.end()){

		windows_.erase(window);

	}
	
}

weak_ptr<Window> Application::GetWindow(const WindowHandle& handle){

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
		for (auto& window : windows_){

			(window.second)->Update(timer.GetTime());

		}

	}

#else

#error "Unsupported platform"

#endif

}

//////////////////////////////////////// WINDOW //////////////////////////////////////////////

#ifdef _WIN32

/// \brief Internal class used to register window classes under Windows.
class Window::WindowClass{

public:

	/// \brief Class name used by the windows.
	const wchar_t * kWindowClassName = L"GiLibWindow";

	/// \brief Get the WindowClass singleton.
	static WindowClass & GetInstance();

	/// \brief Default destructor
	~WindowClass();

private:

	WindowClass();

	HICON window_icon_;

};

Window::WindowClass & Window::WindowClass::GetInstance(){

	static WindowClass instance;

	return instance;

}

Window::WindowClass::~WindowClass(){

	if (window_icon_ != nullptr){

		DestroyIcon(window_icon_);

		window_icon_ = nullptr;

	}

	UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));

}

Window::WindowClass::WindowClass(){

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
	window_description.lpfnWndProc = [](HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

		auto window = Application::GetInstance().GetWindow(window_handle);

		// Proper receiver
		if (auto temp_window = window.lock()){

			return temp_window->ReceiveMessage(message_id, wparameter, lparameter);

		}

		// Default behavior when no receiver is found.
		return DefWindowProc(window_handle, message_id, wparameter, lparameter);

	};

	SetLastError(0);

	//Attempt to register the class

	if (!RegisterClass(&window_description)){

		THROW(L"Could not register window class.");

	}

}

#endif

void Window::SetTitle(const wstring & title){

#ifdef _WIN32

	SetWindowText(handle_, title.c_str());

#else

#error "Unsupported platform"

#endif

}

void Window::Show(bool show){

#ifdef _WIN32

	ShowWindow(handle_, show ? SW_SHOW : SW_HIDE);

#else

#error "Unsupported platform"

#endif

}

bool Window::IsVisible(){

#ifdef _WIN32

	return IsWindowVisible(GetHandle()) != 0;

#else

#error "Unsupported platform"

#endif

}

void Window::Initialize(){

#ifdef _WIN32

	//Create the window
	handle_ = CreateWindow(WindowClass::GetInstance().kWindowClassName,
						   L"",
						   WS_OVERLAPPEDWINDOW,
						   CW_USEDEFAULT,
						   CW_USEDEFAULT,
						   CW_USEDEFAULT,
						   CW_USEDEFAULT,
						   NULL,
						   NULL,
						   GetModuleHandle(nullptr),
						   NULL);

	if(!handle_){
	
		THROW(std::to_wstring(GetLastError()));

	}

#else

#error "Unsupported platform"

#endif

}

Window::~Window(){

#ifdef _WIN32

	DestroyWindow(handle_);

#else

#error "Unsupported platform"

#endif

}

void Window::Update(const Time& time){

	logic_->Update(time);

}

#ifdef _WIN32

LRESULT Window::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	switch (message_id)
	{
	case WM_CLOSE:{

		//Dispose this window
		gi_lib::Application::GetInstance().DisposeWindow(GetHandle());

		OnClosedEventArgs args{ this };

		on_closed_.Notify(args);

		break;
	}
	case WM_SIZE:{

		//The window has been resized

		OnResizedEventArgs args{ this,
								 LOWORD(lparameter),
								 HIWORD(lparameter) };
				
		on_resized_.Notify(args);

		break;
	
	}
	default:
		break;

	}

	return DefWindowProc(GetHandle(), message_id, wparameter, lparameter);

}

#endif