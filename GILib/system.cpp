#include "system.h"

#include <Windows.h>

#include "exceptions.h"

#if defined(_WIN32) || defined(_WIN64)

#define GI_LIB_WINDOWS

#endif

#ifdef GI_LIB_WINDOWS

const int kUnitLabelLength = 3;
const wchar_t * kExtensionSeparator = L".";
const wchar_t * kPathSeparator = L"\\";

#endif

using namespace gi_lib;

OperatingSystem System::GetOperatingSystem(){

#ifdef GI_LIB_WINDOWS

	return OperatingSystem::WINDOWS;

#else

	//Unsupported OS
	static_assert(false);

#endif 

}

wstring System::GetApplicationPath(){

#ifdef GI_LIB_WINDOWS

	wchar_t path_buffer[MAX_PATH + 1];

	GetModuleFileName(0, path_buffer, sizeof(path_buffer));

	return wstring(path_buffer);

#else

	//Unsupported OS
	static_assert(false);

#endif

}

wstring System::GetApplicationName(bool extension){

	auto  path = GetApplicationPath();

	return path.substr(static_cast<unsigned int>(path.find_last_of(kExtensionSeparator)),
					   static_cast<unsigned int>(path.find_last_of(kPathSeparator)));

}

CpuProfile System::GetCPUProfile(){

#ifdef GI_LIB_WINDOWS

	CpuProfile cpu_profile;

	LARGE_INTEGER frequency;
	SYSTEM_INFO system_info;

	if (!QueryPerformanceFrequency(&frequency)){

		throw RuntimeException(L"Your system does not support high-resolution performance counter");

	}

	GetSystemInfo(&system_info);

	cpu_profile.cores = system_info.dwNumberOfProcessors;
	cpu_profile.frequency = frequency.QuadPart * 1000;

	return cpu_profile;

#else

	//Unsupported OS
	static_assert(false);

#endif

}

MemoryProfile System::GetMemoryProfile(){

#ifdef GI_LIB_WINDOWS

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

	//Unsupported OS
	static_assert(false);

#endif

}

StorageProfile System::GetStorageProfile(){

#ifdef GI_LIB_WINDOWS

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

	//Unsupported OS
	static_assert(false);

#endif

}

DesktopProfile System::GetDesktopProfile(){

#ifdef GI_LIB_WINDOWS

	DesktopProfile desktop_profile;

	RECT desktop_rectangle;
	HWND desktop_handle = GetDesktopWindow();

	if (!GetWindowRect(desktop_handle, &desktop_rectangle)){

		throw RuntimeException(L"Invalid argument exception");

	}

	desktop_profile.width = desktop_rectangle.right;
	desktop_profile.height = desktop_rectangle.bottom;

	return desktop_profile;

#else

	//Unsupported OS
	static_assert(false);

#endif

}