#include "application.h"

#include <string>

#ifdef _WIN32

#include <Windows.h>

#endif

#include "system.h"

using namespace gi_lib;
using namespace std;

#ifdef _WIN32

const int kUnitLabelLength = 3;
const wchar_t * kExtensionSeparator = L".";
const wchar_t * kPathSeparator = L"\\";

#endif

wstring Application::GetPath() const{

#ifdef _WIN32

	wchar_t path_buffer[MAX_PATH + 1];

	GetModuleFileName(0, path_buffer, sizeof(path_buffer));

	return wstring(path_buffer);

#else

	//Unsupported OS
	static_assert(false);

#endif

}

wstring Application::GetName(bool extension) const{

	auto  path = GetPath();

	return path.substr(static_cast<unsigned int>(path.find_last_of(kExtensionSeparator)),
					   static_cast<unsigned int>(path.find_last_of(kPathSeparator)));

}