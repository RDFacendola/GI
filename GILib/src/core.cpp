#include "..\include\core.h"

#include <streambuf>
#include <fstream>
#include <algorithm>

#include "..\include\exceptions.h"
#include "..\include\timer.h"

// TODO: Remove this!
#include "..\include\windows\os_windows.h"

using namespace gi_lib;
using namespace std;

#ifdef _WIN32

#include <PathCch.h>

#pragma comment(lib, "Pathcch")

#endif

/////////////////////////////////// GLOBALS /////////////////////////////////////////

#ifdef _WIN32

const wstring gi_lib::kExtensionSeparator = L".";

const wstring gi_lib::kPathSeparator = L"\\";

const unsigned int gi_lib::kUnitLabelLength = 3;

#endif

//////////////////////////////////// SYSTEM /////////////////////////////////////////

OperatingSystem System::GetOperatingSystem(){

#ifdef _WIN32

	return OperatingSystem::WINDOWS;

#else

#error "Unsupported platform"

#endif 

}

CpuProfile System::GetCPUProfile(){

#ifdef _WIN32

	CpuProfile cpu_profile;

	LARGE_INTEGER frequency;
	SYSTEM_INFO system_info;

	if (!QueryPerformanceFrequency(&frequency)){

		THROW(L"Your system does not support high-resolution performance counter");

	}

	GetSystemInfo(&system_info);

	cpu_profile.cores = system_info.dwNumberOfProcessors;
	cpu_profile.frequency = frequency.QuadPart * 1000;

	return cpu_profile;

#else

#error "Unsupported platform"

#endif

}

MemoryProfile System::GetMemoryProfile(){

#ifdef _WIN32

	MemoryProfile memory_profile;

	MEMORYSTATUSEX memory_status;

	memory_status.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memory_status);

	memory_profile.total_physical_memory = memory_status.ullTotalPhys;
	memory_profile.total_virtual_memory = memory_status.ullTotalVirtual;
	memory_profile.total_page_memory = memory_status.ullTotalPageFile;
	memory_profile.available_physical_memory = memory_status.ullAvailPhys;
	memory_profile.available_virtual_memory = memory_status.ullAvailVirtual;
	memory_profile.available_page_memory = memory_status.ullAvailPageFile;

	return memory_profile;

#else

#error "Unsupported platform"

#endif

}

StorageProfile System::GetStorageProfile(){

#ifdef _WIN32

	StorageProfile storage_profile;

	unsigned long drive_mask = GetLogicalDrives();
	wchar_t unit_letter = L'A';

	ULARGE_INTEGER size, available_space;
	DriveProfile drive_profile;

	while (drive_mask != 0){

		drive_profile.unit_letter = wstring(1, unit_letter).append(L":\\");

		if ((drive_mask & 1) &&
			GetDriveType(drive_profile.unit_letter.c_str()) == DRIVE_FIXED){

			GetDiskFreeSpaceEx(drive_profile.unit_letter.c_str(), NULL, &size, &available_space);

			drive_profile.size = size.QuadPart;
			drive_profile.available_space = available_space.QuadPart;

			storage_profile.fixed_drives.push_back(drive_profile);

		}

		drive_mask >>= 1;
		unit_letter++;

	}

	return storage_profile;

#else

#error "Unsupported platform"

#endif

}

DesktopProfile System::GetDesktopProfile(){

#ifdef _WIN32

	DEVMODE devmode;

	devmode.dmSize = sizeof(DEVMODE);

	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode) == 0){

		THROW(L"Could not get desktop profile.");

	}
		
	DesktopProfile desktop_profile;

	desktop_profile.width = devmode.dmPelsWidth;
	desktop_profile.height = devmode.dmPelsHeight;
	desktop_profile.refresh_rate = devmode.dmDisplayFrequency;
	
	return desktop_profile;

#else

#error "Unsupported platform"

#endif

}

////////////////////// IO ////////////////////////////////////////

string IO::ReadFile(const wstring& file_name){

	std::ifstream file_stream(file_name);
	std::string content;

	// Size of the file
	file_stream.seekg(0, std::ios::end);

	content.reserve(static_cast<size_t>(file_stream.tellg()));

	file_stream.seekg(0, std::ios::beg);

	content.assign((std::istreambuf_iterator<char>(file_stream)),
				    std::istreambuf_iterator<char>());

	return content;

}

////////////////////// APPLICATION ///////////////////////////////

Application::Application(){}

Application::~Application(){}

Application & Application::GetInstance(){

	static Application application;

	return application;

}

wstring Application::GetDirectory(){

#ifdef _WIN32

	auto path = GetPath();

	auto path_index = static_cast<unsigned int>(path.find_last_of(kPathSeparator));

	return path.substr(0, path_index + 1);

#else

#error "Unsupported platform"

#endif

}

wstring Application::GetBaseDirectory(const wstring& file_name){

#ifdef _WIN32

	wstring temp(file_name);

	// This doesn't work, only God knows why
	//auto hr = PathCchRemoveExtension(&temp[0], temp.size());

	// This is done only if an extension was found
	if (PathCchRemoveFileSpec(&temp[0], temp.size()) == S_OK){

		temp.resize(temp.find(L'\0'));
		
		return temp + kPathSeparator;

	}
	else{

		return wstring();

	}
	
#else

#error "Not supported";

#endif

}

wstring Application::GetPath(){

#ifdef _WIN32

	wstring path(MAX_PATH + 1, 0);

	GetModuleFileName(0, 
					  &path[0], 
					  static_cast<DWORD>(path.length()));

	path.erase(std::remove(path.begin(),
						   path.end(),
						   0),
			   path.end());

	path.shrink_to_fit();

	return path;

#else

#error "Unsupported platform"

#endif

}

wstring Application::GetName(bool extension){

	auto path = GetPath();

	auto path_index = static_cast<unsigned int>(path.find_last_of(kPathSeparator));
	auto extension_index = static_cast<unsigned int>(path.find_last_of(kExtensionSeparator));

	return path.substr(path_index + 1,
					   extension ? path.npos : extension_index - path_index - 1);

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

Window::Window(){

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