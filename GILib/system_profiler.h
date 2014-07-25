#pragma once

#include <string>
#include <vector>
#include <d3d11.h>

#include "input_range.h"

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

/// Describes a screen resolution
struct RESOLUTION{

	unsigned int width;
	unsigned int height;

};

/// Describes a refresh rate
struct REFRESH_RATE{

	unsigned int numerator;
	unsigned int denominator;

	float GetHz(){

		return static_cast<float>(numerator) / static_cast<float>(denominator);

	}

};

/// Describes a multisample combination
struct MULTISAMPLE{

	unsigned int count;
	unsigned int quality;

};

/// Describes the video mode
struct VIDEO_MODE{

	RESOLUTION resolution;
	REFRESH_RATE refresh_rate;

};

/// Describes a video card capabilities and parameters
struct ADAPTER_PROFILE{

	size_t dedicated_memory;
	size_t shared_memory;
	wstring model_name;
	D3D_FEATURE_LEVEL directx_version;
	vector<const VIDEO_MODE> supported_video_modes;
	vector<const MULTISAMPLE> supported_multisampling;

};

/// Describes the desktop
struct DESKTOP_PROFILE{

	RESOLUTION resolution;

};

/// Utility class to enumerate the system profile
class SystemProfiler{

public:

	static const int kUnitLabelLength;

	/// Get the CPU profile
	/** \param profile The returned cpu profile */
	static void GetCPUProfileOrDie(CPU_PROFILE & profile);

	/// Get the memory profile
	/** \param profile The returned memory profile */
	static void GetMemoryProfile(MEMORY_PROFILE & profile);

	/// Gets the profile of all storage media
	static void GetStorageProfile(STORAGE_PROFILE & profile);

	/// Get the default adapter's profile
	/** \param profile The returned adapter profile */
	static void GetAdapterProfileOrDie(ADAPTER_PROFILE & profile);

	/// Get the desktop's profile
	/** \param profile The returned desktop profile */
	static void GetDesktopProfile(DESKTOP_PROFILE & profile);

private:

	SystemProfiler(){}

	/// Enumerate the multisampling capabilities of the current adapter
	/** \param feature_level The feature level the enumeration refers to
	\param profile Filled with the enumerated capabilities */
	static void EnumerateMultisamplingOrDie(D3D_FEATURE_LEVEL feature_level, ADAPTER_PROFILE & profile);

	/// Enumerate the video modes for the specified adapter
	/** \param adapter Pointer to the adapter to current adapter
	\param profile Filled with the enumerated video modes */
	static void EnumerateVideoModesOrDie(IDXGIAdapter * adapter, ADAPTER_PROFILE & profile);

};