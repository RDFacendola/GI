#include "window.h"

#include "exceptions.h"
#include "application.h"

using namespace gi_lib;

namespace{

#ifdef _WIN32

	// Shared methods under windows OS
	class WindowsShared{

	public:
		
		static const wchar_t * kWindowClassName;

		static WindowsShared & GetInstance(){

			static WindowsShared instance;

			return instance;

		}

		~WindowsShared(){

			if (window_icon_ != nullptr){

				DestroyIcon(window_icon_);

				window_icon_ = nullptr;

			}

			UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));

		}

	private:

		WindowsShared(){
		
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
			window_description.lpfnWndProc = ReceiveMessage;

			SetLastError(0);

			//Attempt to register the class

			THROW_ON_ERROR(RegisterClass(&window_description));

		}

		static LRESULT __stdcall ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

			auto window = Application::GetInstance().GetWindow(window_handle);

			// Proper receiver
			if (auto temp_window = window.lock()){

				return temp_window->ReceiveMessage(message_id, wparameter, lparameter);

			}

			// Default behavior when no receiver is found.
			return DefWindowProc(window_handle, message_id, wparameter, lparameter);

		}

		HICON window_icon_;

	};

	const wchar_t * WindowsShared::kWindowClassName = L"GiLibWindow";

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

	ShowWindow(handle_, show ? SW_SHOW : SW_HIDE);

#else

	static_assert(false, "Unsupported OS");

#endif

}

bool Window::IsVisible(){

#ifdef _WIN32

	return IsWindowVisible(GetHandle()) != 0;

#else

	static_assert(false, "Unsupported OS");

#endif

}

Window::Window(){

#ifdef _WIN32

	//Create the window
	THROW_ON_ERROR(handle_ = CreateWindow(WindowsShared::kWindowClassName,
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

#ifdef _WIN32

LRESULT Window::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	switch (message_id)
	{
	case WM_CLOSE:

		//Dispose this window
		gi_lib::Application::GetInstance().DisposeWindow(GetHandle());

		on_closed_.Notify(*this);

		break;

	case WM_SIZE:

		//The window has been resized
		on_resized_.Notify(*this,
			LOWORD(lparameter),
			HIWORD(lparameter));

		break;

	default:
		break;

	}

	return DefWindowProc(GetHandle(), message_id, wparameter, lparameter);

}

#endif