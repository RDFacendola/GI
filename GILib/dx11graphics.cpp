#include "dx11graphics.h"	

#include <algorithm>

#include "exceptions.h"
#include "raii.h"

const unsigned int kPrimaryDisplayIndex = 0;

const unsigned int kDefaultAdapterIndex = 0;

IDXGIAdapter * const kDefaultAdapter = nullptr;

const unsigned int kMinimumResolution = 1024 * 768;

const DXGI_FORMAT DX11Graphics::kGraphicFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

D3D_FEATURE_LEVEL DX11Graphics::GetFeatureLevel() const{

	return D3D_FEATURE_LEVEL_11_0;

}

ADAPTER_PROFILE DX11Graphics::GetAdapterProfile() const{

	ADAPTER_PROFILE adapter_profile;
	DXGI_ADAPTER_DESC adapter_desc;

	IDXGIFactory * dxgi_factory = nullptr;
	IDXGIAdapter * adapter = nullptr;
	
	//RAII - Release the factory at the end
	ReleaseGuard<IDXGIFactory> guard_factory(dxgi_factory);

	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory),
									(void**)(&dxgi_factory)));

	THROW_ON_FAIL(dxgi_factory->EnumAdapters(kPrimaryDisplayIndex, 
											 &adapter));

	//RAII - Release the adapter at the end
	ReleaseGuard<IDXGIAdapter> guard_adapter(adapter);

	THROW_ON_FAIL(adapter->GetDesc(&adapter_desc));

	adapter_profile.dedicated_memory = adapter_desc.DedicatedVideoMemory;
	adapter_profile.shared_memory = adapter_desc.SharedSystemMemory;
	adapter_profile.model_name = wstring(adapter_desc.Description);
	adapter_profile.supported_video_modes = EnumerateVideoModes(adapter);
	adapter_profile.supported_antialiasing = EnumerateAntialiasingModes();

	//
	return adapter_profile;

}

vector<const VIDEO_MODE> DX11Graphics::EnumerateVideoModes(IDXGIAdapter * adapter) const{
	
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

	//Map DXGI_MODE_DESC to VIDEO_MODE
	vector<const VIDEO_MODE> video_modes(dxgi_modes.size());

	std::transform(dxgi_modes.begin(),
				   dxgi_modes.end(),
				   video_modes.begin(),
				   [this](const DXGI_MODE_DESC & mode)
				   {
	
						return DXGIModeToVideoMode(mode);
	
				   });

	return video_modes;

}

vector<const DXGI_MODE_DESC> DX11Graphics::EnumerateDXGIModes(IDXGIAdapter * adapter) const{

	IDXGIOutput * adapter_output = nullptr;
	DXGI_MODE_DESC * output_modes = nullptr;
	unsigned int output_mode_count;

	//RAII
	ReleaseGuard<IDXGIOutput> guard_adapter_output(adapter_output);
	DeleteGuard<DXGI_MODE_DESC> guard_output_modes(output_modes);

	THROW_ON_FAIL(adapter->EnumOutputs(kPrimaryDisplayIndex,
									   &adapter_output));

	THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
													 0,
													 &output_mode_count,
													 nullptr));

	output_modes = new DXGI_MODE_DESC[output_mode_count];

	THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
													  0,
													  &output_mode_count,
													  output_modes));

	vector<const DXGI_MODE_DESC> dxgi_modes;

	for (unsigned int mode_index = 0; mode_index < output_mode_count; mode_index++){

		dxgi_modes.push_back(output_modes[mode_index]);
		
	}

	return dxgi_modes;

}

vector<const ANTIALIASING_MODE> DX11Graphics::EnumerateAntialiasingModes() const{

	vector<const ANTIALIASING_MODE> antialiasing_modes;

	ID3D11Device * device = nullptr;

	ReleaseGuard<ID3D11Device> guard_device(device);

	DXGI_SAMPLE_DESC sample;

	D3D_FEATURE_LEVEL feature_level[]{ GetFeatureLevel() };

	THROW_ON_FAIL(D3D11CreateDevice(kDefaultAdapter,
									D3D_DRIVER_TYPE_HARDWARE, 
									0, 
									0, 
									feature_level, 
									1,
									D3D11_SDK_VERSION, 
									&device, 
									nullptr, 
									nullptr));

	unsigned int sample_quality_max;

	//Samples must be multiple of 2
	for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sample_count *= 2){

		//If the maximum supported quality is 0 the mode is not supported 	
		THROW_ON_FAIL(device->CheckMultisampleQualityLevels(kGraphicFormat,
														    sample_count,
														    &sample_quality_max));

		if (sample_quality_max > 0){
			
			//Add the lowest quality for that amount of samples
			sample.Count = sample_count;
			sample.Quality = 0;

			antialiasing_modes.push_back(SampleToAntialiasing(sample));

			//Increase the quality exponentially through the maximum value
			for (unsigned int current_quality = 1; current_quality < sample_quality_max; current_quality *= 2){

				sample.Quality = current_quality;

				antialiasing_modes.push_back(SampleToAntialiasing(sample));

			}

		}

	}

	//Remove UNKNOWN stuffs
	antialiasing_modes.erase(std::remove(antialiasing_modes.begin(),
										 antialiasing_modes.end(),
										 ANTIALIASING_MODE::UNKNOWN),
							 antialiasing_modes.end());

	return antialiasing_modes;

}

/// Convert a multisample structure to an antialiasing mode
ANTIALIASING_MODE DX11Graphics::SampleToAntialiasing(const DXGI_SAMPLE_DESC & sample) const{

	if (sample.Count == 1 && sample.Quality == 0)		return ANTIALIASING_MODE::NONE;
	if (sample.Count == 2 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_2X;
	if (sample.Count == 4 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_4X;
	if (sample.Count == 8 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_8X;
	if (sample.Count == 16 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_16X;

	return ANTIALIASING_MODE::UNKNOWN;

}

/// Convert an antialiasing mode to a multisample structure
DXGI_SAMPLE_DESC DX11Graphics::AntialiasingToSample(const ANTIALIASING_MODE & antialiasing) const{

	DXGI_SAMPLE_DESC sample;

	switch (antialiasing)
	{

	case ANTIALIASING_MODE::NONE:

		sample.Count = 1;
		sample.Quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_2X:

		sample.Count = 2;
		sample.Quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_4X:

		sample.Count = 4;
		sample.Quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_8X:

		sample.Count = 8;
		sample.Quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_16X:

		sample.Count = 16;
		sample.Quality = 0;
		break;

	}

	return sample;

}

/// Convert a video mode to a dxgi mode
DXGI_MODE_DESC DX11Graphics::VideoModeToDXGIMode(const VIDEO_MODE & video) const{

	DXGI_MODE_DESC dxgi_mode;

	ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));

	dxgi_mode.Width = video.horizontal_resolution;
	dxgi_mode.Height = video.vertical_resolution;
	dxgi_mode.RefreshRate.Denominator = 1000;
	dxgi_mode.RefreshRate.Numerator = static_cast<unsigned int>(video.refresh_rate_Hz * dxgi_mode.RefreshRate.Denominator);
	dxgi_mode.Format = kGraphicFormat;

	return dxgi_mode;

}

/// Convert a dxgi mode to a video mode
VIDEO_MODE DX11Graphics::DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi) const{
	
	VIDEO_MODE video_mode;

	video_mode.horizontal_resolution = dxgi.Width;
	video_mode.vertical_resolution = dxgi.Height;
	video_mode.refresh_rate_Hz = static_cast<unsigned int>(std::round(static_cast<float>(dxgi.RefreshRate.Numerator) / dxgi.RefreshRate.Denominator));

	return video_mode;

}

///Create the graphic interface. Throws on failure
void DX11Graphics::CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode)
{

	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));

	dxgi_desc.BufferCount = 3;	//Triple buffering only
	dxgi_desc.OutputWindow = window_handle;
	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgi_desc.Windowed = true;  //Change this using IDXGISwapChain::SetFullscreenState
	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	
	//Back buffer
	auto dxgi_mode = VideoModeToDXGIMode(graphic_mode.video);

	memcpy_s(&dxgi_desc.BufferDesc,
			 sizeof(dxgi_desc.BufferDesc),
			 &dxgi_mode,
			 sizeof(dxgi_mode));

	//Antialiasing
	auto dxgi_sample = AntialiasingToSample(graphic_mode.antialiasing);

	memcpy_s(&dxgi_desc.SampleDesc,
			 sizeof(dxgi_desc.SampleDesc),
			 &dxgi_sample,
			 sizeof(dxgi_sample));
	
	D3D_FEATURE_LEVEL feature_levels[] = { GetFeatureLevel() };

	//Create the device and the swapchain
	THROW_ON_FAIL(D3D11CreateDeviceAndSwapChain(kDefaultAdapter, 
												D3D_DRIVER_TYPE_HARDWARE, 
												nullptr, 
												0, 
												feature_levels, 
												1, 
												D3D11_SDK_VERSION, 
												&dxgi_desc, 
												&swap_chain_, 
												&device_, 
												nullptr, 
												&immediate_context_));

	//Save the backbuffer view

	ID3D11Texture2D * backbuffer = nullptr;

	ReleaseGuard<ID3D11Texture2D> guard(backbuffer);

	THROW_ON_FAIL(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)));

	THROW_ON_FAIL(device_->CreateRenderTargetView(backbuffer, nullptr, &backbuffer_view_));
	
	memcpy_s(&graphic_mode_, sizeof(graphic_mode_), &graphic_mode, sizeof(graphic_mode));

}

void DX11Graphics::Present(){

	swap_chain_->Present(graphic_mode_.vsync ? 1 : 0, 0);
	
}