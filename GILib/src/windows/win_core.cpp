#ifdef _WIN32

#include "..\..\Include\windows\win_core.h"

#include <string>
#include <algorithm>
#include <fstream>

#include "..\..\Include\exceptions.h"
#include "..\..\Include\timer.h"

using namespace std;

using namespace gi_lib::windows;

namespace{
	
	const wstring kExtensionSeparator = L".";

	const wstring kPathSeparator = L"\\";

	const unsigned int kUnitLabelLength = 3;

	/// \brief Used to register the window class.
	class WindowRegisterer{

	public:

		/// \brief Class name used by the windows.
		const wchar_t * kWindowClassName = L"GiLibWindow";

		/// \brief Get the WindowClass singleton.
		static WindowRegisterer& GetInstance();

		/// \brief Default destructor
		~WindowRegisterer();

		/// \brief Instantiate a new window.
		/// \return Returns the handle associated to the new window.
		HWND InstantiateWindow();

	private:

		WindowRegisterer();

		HICON window_icon_;		///< \brief Icon associated to the window.

	};

	WindowRegisterer& WindowRegisterer::GetInstance(){

		static WindowRegisterer instance;

		return instance;

	}

	WindowRegisterer::~WindowRegisterer(){

		if (window_icon_ != nullptr){

			DestroyIcon(window_icon_);

			window_icon_ = nullptr;

		}

		UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));

	}

	WindowRegisterer::WindowRegisterer(){

		auto instance = GetModuleHandle(nullptr);

		//Extract the first icon found into the executable
		window_icon_ = ExtractIcon(instance,
								   gi_lib::windows::Application::GetInstance().GetPath().c_str(), 0);

		WNDCLASS window_description;

		ZeroMemory(&window_description, sizeof(WNDCLASS));

		window_description.lpszClassName = kWindowClassName;
		window_description.style = CS_VREDRAW | CS_HREDRAW;
		window_description.hIcon = window_icon_;
		window_description.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_description.hInstance = instance;
		window_description.lpfnWndProc = [](HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

			auto window = gi_lib::windows::Application::GetInstance().GetWindow(window_handle);

			// Fall back to a default handler if no window is found.

			return window ?
				   window->ReceiveMessage(message_id, wparameter, lparameter) :
				   DefWindowProc(window_handle, message_id, wparameter, lparameter);
			
		};

		SetLastError(0);

		//Attempt to register the class

		if (!RegisterClass(&window_description)){

			THROW(L"Could not register window class.");

		}

	}

	HWND WindowRegisterer::InstantiateWindow(){

		return CreateWindow(kWindowClassName,
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

	}

}

//////////////////////////////////// SYSTEM /////////////////////////////////////////

System& System::GetInstance(){

	static System instance;

	return instance;

}

System::System(){}

gi_lib::OperatingSystem System::GetOperatingSystem() const{

	return OperatingSystem::Windows;

}

gi_lib::CpuProfile System::GetCPUProfile() const{

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

}

gi_lib::MemoryProfile System::GetMemoryProfile() const{

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

}

gi_lib::StorageProfile System::GetStorageProfile() const{

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

}

gi_lib::DesktopProfile System::GetDesktopProfile() const{

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

}

//////////////////////////////////// FILESYSTEM //////////////////////////////////////////

FileSystem& FileSystem::GetInstance(){

	static FileSystem instance;

	return instance;

}

FileSystem::FileSystem(){}

wstring FileSystem::GetDirectory(const wstring& file_name) const{

	static wchar_t* separators = L"\\/:";

	auto index = file_name.find_last_of(separators);

	return index != wstring::npos ?
		   file_name.substr(0, index + 1) :
		   file_name;

}

wstring FileSystem::Read(const wstring& file_name) const{

	std::wifstream file_stream(file_name);

	std::wstring content;

	if (file_stream.good()){

		// Reserve the size of the file
		file_stream.seekg(0, std::ios::end);

		content.reserve(static_cast<size_t>(file_stream.tellg()));

		file_stream.seekg(0, std::ios::beg);

		// Copy the content of the file.
		content.assign((std::istreambuf_iterator<wchar_t>(file_stream)),
						std::istreambuf_iterator<wchar_t>());

	}
		
	return content;

}

//////////////////////////////////// APPLICATION /////////////////////////////////////////

Application& Application::GetInstance(){

	static Application instance;

	return instance;

}

Application::Application(){}

wstring Application::GetPath() const{

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

}

wstring Application::GetDirectory() const{

	auto path = GetPath();

	auto path_index = static_cast<unsigned int>(path.find_last_of(kPathSeparator));

	return path.substr(0, path_index + 1);

}

void Application::Join(){
	
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

}

Window& Application::InstantiateWindow(unique_ptr<gi_lib::IWindowLogic> logic){

	auto window_ptr = make_unique<Window>(std::move(logic));

	auto& window = *window_ptr;

	windows_[window.GetHandle()] = std::move(window_ptr);

	return window;
	
}

//////////////////////////////////// WINDOW /////////////////////////////////////////

Window::Window(unique_ptr<gi_lib::IWindowLogic> logic) :
gi_lib::Window(std::move(logic)){

	auto& registerer = WindowRegisterer::GetInstance();

	handle_ = registerer.InstantiateWindow();

	if (!handle_){

		THROW(std::to_wstring(GetLastError()));

	}

}

Window::~Window(){

	DestroyWindow(handle_);

}

void Window::SetTitle(const wstring & title){

	SetWindowText(handle_, 
				  title.c_str());

}

void Window::Show(bool show){

	ShowWindow(handle_, 
			   show ? SW_SHOW : SW_HIDE);

}

bool Window::IsVisible(){

	return IsWindowVisible(handle_) != 0;

}

void Window::Destroy(){

	//Dispose this window
	gi_lib::windows::Application::GetInstance().DisposeWindow(handle_);

}

LRESULT Window::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	switch (message_id)
	{

	case WM_CLOSE:{

		OnClosedEventArgs args{ this };

		on_closed_.Notify(args);

		Destroy();

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

	return DefWindowProc(handle_, 
						 message_id, 
						 wparameter, 
						 lparameter);

}

#endif