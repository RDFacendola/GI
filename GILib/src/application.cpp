#include "application.h"	

#include <string>
#include <algorithm>

#ifdef _WIN32

#include <Windows.h>

#endif

#include "timer.h"
#include "exceptions.h"

using namespace gi_lib;
using namespace std;

namespace{

#ifdef _WIN32

	const int kUnitLabelLength = 3;
	const wchar_t * kExtensionSeparator = L".";
	const wchar_t * kPathSeparator = L"\\";

#endif

}

Application::Application(){}

Application::~Application(){}

Application & Application::GetInstance(){

	static Application application;

	return application;

}

wstring Application::GetPath() const{

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

wstring Application::GetName(bool extension) const{

	auto path = GetPath();

	auto path_index = static_cast<unsigned int>(path.find_last_of(kPathSeparator));
	auto extension_index = static_cast<unsigned int>(path.find_last_of(kExtensionSeparator));

	return path.substr(path_index + 1,
					   extension ? path.npos : extension_index - path_index - 1);

}

void Application::DisposeWindow(const WindowHandle & handle){

	//The shared pointer's destructor will take care of destroying the window correctly
	auto window = windows_.find(handle);

	if (window != windows_.end()){

		windows_.erase(window);

	}
	
}

weak_ptr<Window> Application::GetWindow(const WindowHandle & handle){

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

#error "Unsupported platform"

#endif

}