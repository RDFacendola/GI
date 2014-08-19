#include "window.h"

#include "exceptions.h"
#include "application.h"

using namespace gi_lib;

#ifdef _WIN32

	// Inner class of Window, used internally.
	class Window::WindowClass{

	public:
		
		const wchar_t * kWindowClassName = L"GiLibWindow";

		static WindowClass & GetInstance(){

			static WindowClass instance;

			return instance;

		}

		~WindowClass(){

			if (window_icon_ != nullptr){

				DestroyIcon(window_icon_);

				window_icon_ = nullptr;

			}

			UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));

		}

	private:

		WindowClass(){
		
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

			THROW_ON_ERROR(RegisterClass(&window_description));

		}

		HICON window_icon_;

	};

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

Window::Window(){

#ifdef _WIN32

	//Create the window
	THROW_ON_ERROR(handle_ = CreateWindow(WindowClass::GetInstance().kWindowClassName,
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