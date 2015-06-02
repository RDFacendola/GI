#ifdef _WIN32

#include "..\..\Include\windows\win_core.h"

#include <Windows.h>

#include "..\..\Include\exceptions.h"

using namespace gi_lib::windows;

//////////////////////////////////// SYSTEM /////////////////////////////////////////

gi_lib::OperatingSystem System::GetOperatingSystem(){

	return OperatingSystem::WINDOWS;

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

#endif