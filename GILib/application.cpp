#include <exception>
#include <string>

#include "application.h"
#include "services.h"
#include "exceptions.h"

using ::std::wstring;

Application & Application::GetSingleton(){

	static Application singleton;

	return singleton;

}

unsigned int Application::kApplicationLogicMessage = 0x0401;

unsigned int Application::kApplicationTimeMessage = 0x0402;

void Application::Run(HINSTANCE application_instance, IApplicationLogic & application_logic){

	MSG message;
	Timer timer;
	HICON icon_handle = 0x0;
	HWND window_handle = 0x0;
	APPLICATION_TIME application_time;

	float time;

	try{

		//Initialization

		window_handle = CreateApplicationWindow(application_instance, application_logic);

		application_logic.Initialize(window_handle);
		
		SendMessage(window_handle, kApplicationLogicMessage, 0, reinterpret_cast<LPARAM>(&application_logic));
		SendMessage(window_handle, kApplicationTimeMessage, 0, reinterpret_cast<LPARAM>(&application_time));

		//Main loop
		while (application_logic.IsRunning()){

			//Update the application time
			time = timer.GetTime();

			application_time.deltaSeconds = time - application_time.totalSeconds;
			application_time.totalSeconds = time;
			
			while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) == TRUE){

				if (message.message == WM_QUIT){

					application_logic.Exit();

				}

				//If we have a message call the proper routine
				TranslateMessage(&message);
				DispatchMessage(&message);

			}

			//If no message is received, update the logic
			application_logic.Update(window_handle, application_time);

		}

		//Clean everything up
		CleanUp(window_handle, icon_handle, application_logic);

	}
	catch (...){

		//Something went terribly wrong: destroy everything an rethrow the exception
		CleanUp(window_handle, icon_handle, application_logic);

		throw;

	}

}

void Application::CleanUp(HWND window_handle, HICON icon_handle, IApplicationLogic & application_logic){

	application_logic.Destroy();

	SendMessage(window_handle, kApplicationLogicMessage, 0, reinterpret_cast<LPARAM>(nullptr));
	SendMessage(window_handle, kApplicationTimeMessage, 0, reinterpret_cast<LPARAM>(nullptr));

	DestroyWindow(window_handle);
	DestroyIcon(icon_handle);

}

LRESULT __stdcall Application::ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	static IApplicationLogic * application_logic = nullptr;
	static APPLICATION_TIME * application_time = nullptr;

	if (message_id == kApplicationLogicMessage){

		application_logic = reinterpret_cast<IApplicationLogic *>(lparameter);
		return 0;

	}
	else if (message_id == kApplicationTimeMessage){

		application_time = reinterpret_cast<APPLICATION_TIME *>(lparameter);;
		return 0;

	}

	//Just relay the message to the running application logic
	if (application_logic == nullptr &&
		application_time == nullptr){

		return DefWindowProc(window_handle, message_id, wparameter, lparameter);

	}
	else{

		if (message_id == WM_CLOSE){

			PostQuitMessage(0);

		}

		return application_logic->ReceiveMessage(window_handle, message_id, wparameter, lparameter, *application_time);

	}

}

HWND Application::CreateApplicationWindow(HINSTANCE application_instance, const IApplicationLogic & application_logic){

	HICON icon_handle = ExtractIcon(application_instance, Services::GetApplicationPath().c_str(), 0);	//Extract the first icon found into the executable

	wstring application_name = Services::GetApplicationName();

	//Register the window class
	WNDCLASS window_description;

	ZeroMemory(&window_description, sizeof(WNDCLASS));

	window_description.lpszClassName = application_name.c_str();
	window_description.style = CS_VREDRAW | CS_HREDRAW;
	window_description.hIcon = icon_handle;
	window_description.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_description.hInstance = application_instance;
	window_description.lpfnWndProc = ReceiveMessage;	//Callback for windows messages

	//Attempt to register the class
	if (!RegisterClass(&window_description)){

		throw RuntimeException(L"Unable to register the window class");

	}

	//Create the window
	HWND window_handle = CreateWindow(application_name.c_str(),
								 	  application_logic.GetWindowTitle().c_str(),
									  WS_OVERLAPPEDWINDOW,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  application_logic.GetWindowWidth(),
									  application_logic.GetWindowHeight(),
									  NULL,
									  NULL,
									  application_instance,
									  NULL);

	if (!window_handle){

		throw RuntimeException(L"Unable to create the window");

	}

	//Ok
	ShowWindow(window_handle, SW_SHOWDEFAULT);

	return window_handle;

}