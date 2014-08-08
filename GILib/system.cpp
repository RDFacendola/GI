#include "system.h"

#include <Windows.h>

#include "exceptions.h"

using namespace gi_lib;

OperatingSystem System::GetOperatingSystem(){

#ifdef _WIN32

	return OperatingSystem::WINDOWS;

#else

	static_assert(false, "Unsupported OS");

#endif 

}

CpuProfile System::GetCPUProfile(){

#ifdef _WIN32

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

	static_assert(false, "Unsupported OS");

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

	static_assert(false, "Unsupported OS");

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

	static_assert(false, "Unsupported OS");

#endif

}

DesktopProfile System::GetDesktopProfile(){

#ifdef _WIN32

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

	static_assert(false, "Unsupported OS");

#endif

}