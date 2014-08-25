#include <d3d11.h>

#include "dx11/dx11factory.h"
#include "dx11/dx11graphics.h"
#include "dx11/dx11resources.h"
#include "dx11/dx11shared.h"
#include "exceptions.h"
#include <algorithm>
#include <memory>

using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace std;

namespace{

	/// Index of the primary output.
	const unsigned int kPrimaryOutputIndex = 0;

	/// Index of the default video card.
	const unsigned int kDefaultAdapterIndex = 0;

	/// Pointer to the default video card.
	IDXGIAdapter * const kDefaultAdapter = nullptr;

	/// Number of buffers used by the swapchain.
	const unsigned int kBuffersCount = 3;

	/// Minimum resolution allowed, in pixels.
	const unsigned int kMinimumResolution = 1024 * 768;

	/// Surface's format.
	const DXGI_FORMAT kGraphicFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	/// DirectX 11 API support.
	const D3D_FEATURE_LEVEL kFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	/// Convert a multisample structure to an antialiasing mode
	AntialiasingMode SampleDescToAntialiasingMode(const DXGI_SAMPLE_DESC & sample_desc){

		if (sample_desc.Count == 1 && sample_desc.Quality == 0)		return AntialiasingMode::NONE;
		if (sample_desc.Count == 2 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_2X;
		if (sample_desc.Count == 4 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_4X;
		if (sample_desc.Count == 8 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_8X;
		if (sample_desc.Count == 16 && sample_desc.Quality == 0)	return AntialiasingMode::MSAA_16X;

		return AntialiasingMode::NONE;

	}

	/// Convert an antialiasing mode to a multisample structure
	DXGI_SAMPLE_DESC AntialiasingModeToSampleDesc(const AntialiasingMode & antialiasing_mode){

		DXGI_SAMPLE_DESC sample_desc;

		switch (antialiasing_mode)
		{

		case AntialiasingMode::MSAA_2X:

			sample_desc.Count = 2;
			sample_desc.Quality = 0;
			break;

		case AntialiasingMode::MSAA_4X:

			sample_desc.Count = 4;
			sample_desc.Quality = 0;
			break;

		case AntialiasingMode::MSAA_8X:

			sample_desc.Count = 8;
			sample_desc.Quality = 0;
			break;

		case AntialiasingMode::MSAA_16X:

			sample_desc.Count = 16;
			sample_desc.Quality = 0;
			break;

		case AntialiasingMode::NONE:
		default:

			sample_desc.Count = 1;
			sample_desc.Quality = 0;
			break;

		}

		return sample_desc;

	}

	/// Convert a video mode to a mode desc
	DXGI_MODE_DESC VideoModeToDXGIMode(const VideoMode & video_mode){

		DXGI_MODE_DESC dxgi_mode;

		ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));

		dxgi_mode.Width = video_mode.horizontal_resolution;
		dxgi_mode.Height = video_mode.vertical_resolution;
		dxgi_mode.RefreshRate.Denominator = 1000;
		dxgi_mode.RefreshRate.Numerator = static_cast<unsigned int>(video_mode.refresh_rate * dxgi_mode.RefreshRate.Denominator);
		dxgi_mode.Format = kGraphicFormat;

		return dxgi_mode;

	}

	/// Convert a mode desc to a video mode
	VideoMode DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi_mode){

		VideoMode video_mode;

		video_mode.horizontal_resolution = dxgi_mode.Width;
		video_mode.vertical_resolution = dxgi_mode.Height;
		video_mode.refresh_rate = static_cast<unsigned int>(std::round(static_cast<float>(dxgi_mode.RefreshRate.Numerator) / dxgi_mode.RefreshRate.Denominator));

		return video_mode;

	}

	/// Enumerate the supported DXGI video modes
	vector<DXGI_MODE_DESC> EnumerateDXGIModes(IDXGIAdapter & adapter){

		IDXGIOutput * adapter_output = nullptr;
		unsigned int output_mode_count;

		THROW_ON_FAIL(adapter.EnumOutputs(kPrimaryOutputIndex,
			&adapter_output));

		//Release the output when the function returns or throws
		auto guard = unique_ptr<IDXGIOutput, COMDeleter>(adapter_output);

		THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
			0,
			&output_mode_count,
			nullptr));

		vector<DXGI_MODE_DESC> dxgi_modes(output_mode_count);

		THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
			0,
			&output_mode_count,
			&dxgi_modes[0]));

		return dxgi_modes;

	}

	/// Enumerate the supported video modes. Filters by resolution and refresh rate
	vector<VideoMode> EnumerateVideoModes(IDXGIAdapter & adapter){

		auto dxgi_modes = EnumerateDXGIModes(adapter);

		//Remove the modes that do not satisfy the minimum requirements
		dxgi_modes.erase(std::remove_if(dxgi_modes.begin(),
			dxgi_modes.end(),
			[](const DXGI_MODE_DESC & value){

			return value.Width * value.Height < kMinimumResolution;

		}),
			dxgi_modes.end());

		//Sorts the modes by width, height and refresh rate
		std::sort(dxgi_modes.begin(),
			dxgi_modes.end(),
			[](const DXGI_MODE_DESC & first, const DXGI_MODE_DESC & second){

			return first.Width < second.Width ||
				first.Width == second.Width &&
				(first.Height < second.Height ||
				first.Height == second.Height &&
				first.RefreshRate.Numerator * second.RefreshRate.Denominator > second.RefreshRate.Numerator * first.RefreshRate.Denominator);

		});

		//Keeps the highest refresh rate for each resolution combination and discards everything else
		dxgi_modes.erase(std::unique(dxgi_modes.begin(),
			dxgi_modes.end(),
			[](const DXGI_MODE_DESC & first, const DXGI_MODE_DESC & second){

			return first.Width == second.Width &&
				first.Height == second.Height;

		}),
			dxgi_modes.end());

		//Map DXGI_MODE_DESC to VideoMode
		vector<VideoMode> video_modes(dxgi_modes.size());

		std::transform(dxgi_modes.begin(),
			dxgi_modes.end(),
			video_modes.begin(),
			[](const DXGI_MODE_DESC & mode)
		{

			return DXGIModeToVideoMode(mode);

		});

		return video_modes;

	}

	/// Enumerate the supported antialiasing modes
	vector<AntialiasingMode> EnumerateAntialiasingModes(ID3D11Device & device){

		vector<AntialiasingMode> antialiasing_modes;

		DXGI_SAMPLE_DESC sample;

		unsigned int sample_quality_max;

		AntialiasingMode antialiasing_mode;

		for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++sample_count){

			//If the maximum supported quality is 0 the mode is not supported 	
			THROW_ON_FAIL(device.CheckMultisampleQualityLevels(kGraphicFormat,
				sample_count,
				&sample_quality_max));

			if (sample_quality_max > 0){

				//Add the lowest quality for that amount of samples
				sample.Count = sample_count;
				sample.Quality = 0;

				antialiasing_modes.push_back(SampleDescToAntialiasingMode(sample));

				//Increase the quality exponentially through the maximum value
				for (unsigned int current_quality = 1; current_quality < sample_quality_max; current_quality *= 2){

					sample.Quality = current_quality;

					antialiasing_mode = SampleDescToAntialiasingMode(sample);

					//Discard unknown modes
					if (antialiasing_mode != AntialiasingMode::NONE){

						antialiasing_modes.push_back(antialiasing_mode);

					}

				}

			}

		}

		return antialiasing_modes;

	}

}

DX11Factory & DX11Factory::GetInstance(){

	static DX11Factory factory;

	return factory;

}

DX11Factory::DX11Factory() :
device_(nullptr){

	//DXGI Factory
	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory),
		(void**)(&factory_)));

	//DXGI Adapter
	THROW_ON_FAIL(factory_->EnumAdapters(kPrimaryOutputIndex,
		&adapter_));

	//DXGI Device
	THROW_ON_FAIL(D3D11CreateDevice(kDefaultAdapter,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		0,
		&kFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&device_,
		nullptr,
		nullptr));

}

DX11Factory::~DX11Factory(){

	device_->Release();
	adapter_->Release();
	factory_->Release();

}

AdapterProfile DX11Factory::GetAdapterProfile() const{

	AdapterProfile adapter_profile;
	DXGI_ADAPTER_DESC adapter_desc;

	THROW_ON_FAIL(adapter_->GetDesc(&adapter_desc));

	adapter_profile.name = wstring(adapter_desc.Description);
	adapter_profile.dedicated_memory = adapter_desc.DedicatedVideoMemory;
	adapter_profile.shared_memory = adapter_desc.SharedSystemMemory;
	adapter_profile.video_modes = EnumerateVideoModes(*adapter_);
	adapter_profile.antialiasing_modes = EnumerateAntialiasingModes(*device_);
	
	adapter_profile.max_anisotropy = D3D11_MAX_MAXANISOTROPY;

	//
	return adapter_profile;

}

unique_ptr<Graphics> DX11Factory::CreateGraphics(Window & window){

	return std::make_unique<DX11Graphics>(window, *device_, *factory_);

}

ResourceManager & DX11Factory::GetResourceManager(){

	static DX11ResourceManager resources(*device_);

	return resources;

}