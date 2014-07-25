#include "services.h"

#include <Windows.h>

#include "exceptions.h"

const int kUnitLabelLength = 3;

wstring Services::GetApplicationPath(){

	wchar_t path_buffer[MAX_PATH + 1];

	GetModuleFileName(0, path_buffer, sizeof(path_buffer));

	return wstring(path_buffer);

}

wstring Services::GetApplicationName(bool extension){

	auto  path = GetApplicationPath();

	return path.substr(static_cast<unsigned int>(path.find_last_of(L".")),		//Extension separator
						static_cast<unsigned int>(path.find_last_of(L"\\")));	//Path separator

}

CPU_PROFILE Services::GetCPUProfile(){

	CPU_PROFILE cpu_profile;

	LARGE_INTEGER frequency;
	SYSTEM_INFO system_info;

	if (!QueryPerformanceFrequency(&frequency)){

		throw RuntimeException(L"Your system does not support high-resolution performance counter");

	}

	GetSystemInfo(&system_info);

	cpu_profile.cores = system_info.dwNumberOfProcessors;
	cpu_profile.frequency = frequency.QuadPart * 1000;

	return cpu_profile;

}

MEMORY_PROFILE Services::GetMemoryProfile(){

	MEMORY_PROFILE memory_profile;

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

STORAGE_PROFILE Services::GetStorageProfile(){

	STORAGE_PROFILE storage_profile;

	unsigned long drive_mask = GetLogicalDrives();
	wchar_t unit_letter = L'A';

	ULARGE_INTEGER size, available_space;
	DRIVE_PROFILE drive;

	while (drive_mask != 0){

		drive.label = wstring(1, unit_letter).append(L":\\");

		if ((drive_mask & 1) &&
			GetDriveType(drive.label.c_str()) == DRIVE_FIXED){

			GetDiskFreeSpaceEx(drive.label.c_str(), NULL, &size, &available_space);

			drive.size = size.QuadPart;
			drive.available_space = available_space.QuadPart;

			storage_profile.fixed_drives.push_back(drive);

		}

		drive_mask >>= 1;
		unit_letter++;

	}

	return storage_profile;

}

DESKTOP_PROFILE Services::GetDesktopProfile(){

	DESKTOP_PROFILE desktop_profile;

	RECT desktop_rectangle;
	HWND desktop_handle = GetDesktopWindow();

	if (!GetWindowRect(desktop_handle, &desktop_rectangle)){

		throw RuntimeException(L"Invalid argument exception");

	}

	desktop_profile.width = desktop_rectangle.right;
	desktop_profile.height = desktop_rectangle.bottom;

	return desktop_profile;

}