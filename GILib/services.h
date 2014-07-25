#pragma once

#include <string>
#include <vector>

using ::std::wstring;
using ::std::vector;


/// Describes the CPU
struct CPU_PROFILE{

	unsigned int cores;
	unsigned __int64 frequency;

};

/// Describes a disk
struct DRIVE_PROFILE{

	unsigned long long size;
	unsigned long long available_space;
	wstring label;							///< The letter of the unit

};

/// Describes a storage media
struct STORAGE_PROFILE{

	vector<DRIVE_PROFILE> fixed_drives;

};

/// Describes the memory
struct MEMORY_PROFILE{

	unsigned long long total_physical_memory;
	unsigned long long total_virtual_memory;
	unsigned long long total_page_memory;
	unsigned long long available_physical_memory;
	unsigned long long available_virtual_memory;
	unsigned long long available_page_memory;

};


/// Describes the desktop
struct DESKTOP_PROFILE{

	unsigned int width;
	unsigned int height;

};

///System services
class Services{

public:

	/// Get the full application path
	static wstring GetApplicationPath();

	/// Get the application name
	/** \param extension Set it to true if the name should include the extension, set it to false otherwise */
	static wstring GetApplicationName(bool extension = true);

	/// Get the CPU profile
	static CPU_PROFILE GetCPUProfile();

	/// Get the memory profile
	static MEMORY_PROFILE GetMemoryProfile();

	/// Gets the profile of all storage media
	static STORAGE_PROFILE GetStorageProfile();

	/// Get the desktop's profile
	static DESKTOP_PROFILE GetDesktopProfile();

private:

	Services(){}

};