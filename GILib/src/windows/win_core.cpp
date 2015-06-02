#ifdef _WIN32

#include "..\..\Include\windows\win_core.h"

#include <Windows.h>
#include <string>
#include <algorithm>
#include <fstream>

#include "..\..\Include\exceptions.h"

using namespace std;

using namespace gi_lib::windows;

namespace{
	
	const wstring kExtensionSeparator = L".";

	const wstring kPathSeparator = L"\\";

	const unsigned int kUnitLabelLength = 3;

}

//////////////////////////////////// SYSTEM /////////////////////////////////////////

gi_lib::OperatingSystem System::GetOperatingSystem(){

	return OperatingSystem::Windows;

}

gi_lib::CpuProfile System::GetCPUProfile(){

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

gi_lib::MemoryProfile System::GetMemoryProfile(){

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

gi_lib::StorageProfile System::GetStorageProfile(){

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

gi_lib::DesktopProfile System::GetDesktopProfile(){

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

wstring FileSystem::GetDirectory(const wstring& file_name){

	static wchar_t* separators = L"\\/:";

	auto index = file_name.find_last_of(separators);

	return index != wstring::npos ?
		   file_name.substr(0, index + 1) :
		   file_name;

}

wstring FileSystem::Read(const wstring& file_name){

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

wstring Application::GetPath(){

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

wstring Application::GetDirectory(){

	auto path = GetPath();

	auto path_index = static_cast<unsigned int>(path.find_last_of(kPathSeparator));

	return path.substr(0, path_index + 1);

}

#endif