#include "system_profiler.h"

#include <Windows.h>
#include <assert.h>
#include <d3d11.h>

#include "exceptions.h"
#include "release_guard.h"
#include "delete_guard.h"

#define MINIMUM_HORIZONTAL_RESOLUTION 1024
#define MINIMUM_VERTICAL_RESOLUTION 768

const int SystemProfiler::kUnitLabelLength = 3;

void SystemProfiler::GetCPUProfileOrDie(CPU_PROFILE & profile){

	LARGE_INTEGER frequency;
	SYSTEM_INFO system_info;

	if (!QueryPerformanceFrequency(&frequency)){

		throw RuntimeException(L"Your system does not support high-resolution performance counter");

	}

	GetSystemInfo(&system_info);

	profile.cores = system_info.dwNumberOfProcessors;
	profile.frequency = frequency.QuadPart * 1000;

}

void SystemProfiler::GetMemoryProfile(MEMORY_PROFILE & profile){

	MEMORYSTATUSEX memory_status;

	memory_status.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memory_status);

	profile.total_physical_memory = memory_status.ullTotalPhys;
	profile.total_virtual_memory = memory_status.ullTotalVirtual;
	profile.total_page_memory = memory_status.ullTotalPageFile;
	profile.available_physical_memory = memory_status.ullAvailPhys;
	profile.available_virtual_memory = memory_status.ullAvailVirtual;
	profile.available_page_memory = memory_status.ullAvailPageFile;

}

void SystemProfiler::GetStorageProfile(STORAGE_PROFILE & profile){

	profile.fixed_drives.clear();

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

			profile.fixed_drives.push_back(drive);

		}

		drive_mask >>= 1;
		unit_letter++;

	}

}

void SystemProfiler::EnumerateMultisamplingOrDie(D3D_FEATURE_LEVEL feature_level, ADAPTER_PROFILE & profile){

	ID3D11Device * device;

	ReleaseGuard<ID3D11Device> guard_device(device);

	MULTISAMPLE multisample_combination;

	THROW_ON_FAIL(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &feature_level, 1, D3D11_SDK_VERSION, &device, nullptr, nullptr));

	unsigned int sample_quality_max;

	profile.supported_multisampling.clear();

	//Samples must be multiple of 2
	for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sample_count *= 2){

		THROW_ON_FAIL(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, sample_count, &sample_quality_max));

		//If the quality is 0 the mode is not supported		
		if (sample_quality_max > 0){

			//Add the lowest quality for that amount of samples
			multisample_combination.count = sample_count;
			multisample_combination.quality = 0;

			profile.supported_multisampling.push_back(multisample_combination);

			//Increase the quality exponentially through the maximum value
			for (unsigned int current_quality = 1; current_quality < sample_quality_max; current_quality *= 2){

				multisample_combination.quality = current_quality;

				profile.supported_multisampling.push_back(multisample_combination);

			}

		}

	}
	
}

void SystemProfiler::EnumerateVideoModesOrDie(IDXGIAdapter * adapter, ADAPTER_PROFILE & profile){

	IDXGIOutput * adapter_output;
	DXGI_MODE_DESC * output_modes;
	DXGI_MODE_DESC * current_output_mode;
	unsigned int output_mode_count;
	VIDEO_MODE new_video_mode;
	VIDEO_MODE last_video_mode;

	//RAII
	ReleaseGuard<IDXGIOutput> guard_adapter_output(adapter_output);
	DeleteGuard<DXGI_MODE_DESC> guard_output_modes(output_modes);

	adapter->EnumOutputs(0, &adapter_output);

	THROW_ON_FAIL(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &output_mode_count, nullptr));

	output_modes = new DXGI_MODE_DESC[output_mode_count];

	THROW_ON_FAIL(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &output_mode_count, output_modes));

	profile.supported_video_modes.clear();

	for (unsigned int mode_index = 0; mode_index < output_mode_count; mode_index++){

		current_output_mode = output_modes + mode_index;

		new_video_mode.resolution.width = current_output_mode->Width;
		new_video_mode.resolution.height = current_output_mode->Height;
		new_video_mode.refresh_rate.numerator = current_output_mode->RefreshRate.Numerator;
		new_video_mode.refresh_rate.denominator = current_output_mode->RefreshRate.Denominator;

		//Skips resolution lower than MINIMUM_HORIZONTAL_RESOLUTION x MINIMUM_VERTICAL_RESOLUTION
		if (new_video_mode.resolution.width < MINIMUM_HORIZONTAL_RESOLUTION ||
			new_video_mode.resolution.height < MINIMUM_VERTICAL_RESOLUTION){

			continue;

		}

		//Prevents duplicated video modes
		if (profile.supported_video_modes.size() > 0){

			last_video_mode = profile.supported_video_modes.back();

			if (last_video_mode.resolution.width == new_video_mode.resolution.width &&
				last_video_mode.resolution.height == new_video_mode.resolution.height &&
				last_video_mode.refresh_rate.numerator == new_video_mode.refresh_rate.numerator &&
				last_video_mode.refresh_rate.denominator == new_video_mode.refresh_rate.denominator){

				//Skip to next mode
				continue;

			}

		}

		profile.supported_video_modes.push_back(new_video_mode);

	}
	
}

void SystemProfiler::GetAdapterProfileOrDie(ADAPTER_PROFILE & profile){

	D3D_FEATURE_LEVEL feature_level;
	IDXGIFactory * dxgi_factory;
	IDXGIAdapter * adapter;
	DXGI_ADAPTER_DESC adapter_desc;

	//RAII
	ReleaseGuard<IDXGIFactory> guard_factory(dxgi_factory);
	ReleaseGuard<IDXGIAdapter> guard_adapter(adapter);
	
	static D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1,
												  D3D_FEATURE_LEVEL_11_0,
												  D3D_FEATURE_LEVEL_10_1,
												  D3D_FEATURE_LEVEL_10_0,
												  D3D_FEATURE_LEVEL_9_3,
												  D3D_FEATURE_LEVEL_9_2,
												  D3D_FEATURE_LEVEL_9_1 };

	//Get the supported DirectX version
	auto count = sizeof(feature_levels) / sizeof(D3D_FEATURE_LEVEL);

	THROW_ON_FAIL(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, feature_levels, count, D3D11_SDK_VERSION, nullptr, &feature_level, nullptr));

	//Get the default adapter
	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&dxgi_factory)));

	THROW_ON_FAIL(dxgi_factory->EnumAdapters(0, &adapter));

	THROW_ON_FAIL(adapter->GetDesc(&adapter_desc));

	profile.directx_version = feature_level;
	profile.dedicated_memory = adapter_desc.DedicatedVideoMemory;
	profile.shared_memory = adapter_desc.SharedSystemMemory;
	profile.model_name = wstring(adapter_desc.Description);

	//Enumerates the multisampling capabilities and fills the profile
	EnumerateMultisamplingOrDie(feature_level, profile);

	//Enumerate the video modes and fills the profile
	EnumerateVideoModesOrDie(adapter, profile);
	
}

void SystemProfiler::GetDesktopProfile(DESKTOP_PROFILE & profile){

	RECT desktop_rectangle;
	HWND desktop_handle = GetDesktopWindow();

	if (!GetWindowRect(desktop_handle, &desktop_rectangle)){

		throw RuntimeException(L"Invalid argument exception");

	}

	profile.resolution.width = desktop_rectangle.right;
	profile.resolution.height = desktop_rectangle.bottom;

}