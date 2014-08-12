#ifdef _WIN32

#include "dx11graphics.h"

#include "exceptions.h"
#include "guard.h"
#include "application.h"

#include <algorithm>

using namespace gi_lib;

//Global stuffs

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

}

//DX11Factory

vector<VideoMode> DX11Factory::EnumerateVideoModes() const{

	auto dxgi_modes = EnumerateDXGIModes();

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

vector<DXGI_MODE_DESC> DX11Factory::EnumerateDXGIModes() const{

	IDXGIOutput * adapter_output = nullptr;
	unsigned int output_mode_count;
	
	THROW_ON_FAIL(adapter_->EnumOutputs(kPrimaryOutputIndex,
									    &adapter_output));

	//Release the output when the function returns or throws
	auto guard = MakeScopeGuard([adapter_output]{ adapter_output->Release(); });

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

vector<AntialiasingMode> DX11Factory::EnumerateAntialiasingModes() const{

	vector<AntialiasingMode> antialiasing_modes;

	DXGI_SAMPLE_DESC sample;

	unsigned int sample_quality_max;

	AntialiasingMode antialiasing_mode;

	for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++sample_count){

		//If the maximum supported quality is 0 the mode is not supported 	
		THROW_ON_FAIL(device_->CheckMultisampleQualityLevels(kGraphicFormat,
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

DX11Factory::DX11Factory():
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
	adapter_profile.video_modes = EnumerateVideoModes();
	adapter_profile.antialiasing_modes = EnumerateAntialiasingModes();

	//
	return adapter_profile;

}

shared_ptr<IGraphics> DX11Factory::CreateGraphics(Window & window){

	return std::make_shared<DX11Graphics>(window, *device_, *factory_);

}

//DX11Graphics

DX11Graphics::DX11Graphics(Window & window, ID3D11Device & device, IDXGIFactory & factory) :
	window_(window),
	device_(device),
	factory_(factory),
	swap_chain_(nullptr),
	on_window_resized_(ListenerHandle::kNull){

	///VSync disabled by default
	SetVSync(false);

	//Create the swapchain with default parameters
	CreateSwapChain(GetDefaultSwapchainMode());

	//Listeners
	on_window_resized_ = window_.OnResized().AddListener([this](Window &, unsigned int, unsigned int){

															//Resize the swapchain buffer
															swap_chain_->ResizeBuffers(kBuffersCount,
																					   0,	//Will fit the client width
																					   0,	//Will fit the client height
																					   kGraphicFormat,
																					   0);

														 });

}

///Destroy the graphic object
DX11Graphics::~DX11Graphics(){

	//Return to windowed mode (otherwise the screen will hang)
	SetFullscreen(false);

	swap_chain_->Release();

}

void DX11Graphics::SetVideoMode(const VideoMode & video_mode){

	video_mode_ = video_mode;

	//Resize the client window (this will cause the resize of the backbuffer)
	DXGI_MODE_DESC dxgi_mode;

	ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));

	dxgi_mode.Format = kGraphicFormat;
	dxgi_mode.Width = video_mode.horizontal_resolution;
	dxgi_mode.Height = video_mode.vertical_resolution;
	dxgi_mode.RefreshRate.Denominator = 1000;
	dxgi_mode.RefreshRate.Numerator = video_mode.refresh_rate * dxgi_mode.RefreshRate.Denominator;

	swap_chain_->ResizeTarget(&dxgi_mode);

}

void DX11Graphics::SetAntialisingMode(const AntialiasingMode & antialiasing_mode){

	antialiasing_mode_ = antialiasing_mode;

	//Create the swapchain again (keeps everything except the antialiasing)
	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	swap_chain_->GetDesc(&dxgi_desc);

	dxgi_desc.SampleDesc = AntialiasingModeToSampleDesc(antialiasing_mode);

	CreateSwapChain(dxgi_desc);

}

void DX11Graphics::SetFullscreen(bool fullscreen){

	fullscreen_ = fullscreen;

	swap_chain_->SetFullscreenState(fullscreen,
									nullptr);

}

///Return the default DXGI mode for the swap chain
DXGI_SWAP_CHAIN_DESC DX11Graphics::GetDefaultSwapchainMode() const{

	//Create the swapchain with default settings
	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));

	dxgi_desc.BufferCount = kBuffersCount;
	dxgi_desc.OutputWindow = window_.GetHandle();
	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgi_desc.Windowed = true;
	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//Back buffer and antialiasing - Default
	dxgi_desc.BufferDesc.Format = kGraphicFormat;
	dxgi_desc.SampleDesc.Count = 1;

	return dxgi_desc;

}

///Create a new swapchain given its description
void DX11Graphics::CreateSwapChain(DXGI_SWAP_CHAIN_DESC desc){

	//TODO: Release the outstanding references to the backbuffer in the context

	//Release the old swapchain
	if (swap_chain_ != nullptr){

		swap_chain_->Release();
		swap_chain_ = nullptr;

	}

	THROW_ON_FAIL(factory_.CreateSwapChain(&device_,
										   &desc,
										   &swap_chain_));

	//TODO: Get the references to the backbuffer
	/*

	//Save the backbuffer view

	ID3D11Texture2D * backbuffer = nullptr;

	ReleaseGuard<ID3D11Texture2D> guard(backbuffer);

	THROW_ON_FAIL(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)));

	THROW_ON_FAIL(device_->CreateRenderTargetView(backbuffer, nullptr, &backbuffer_view_));

	memcpy_s(&graphic_mode_, sizeof(graphic_mode_), &graphic_mode, sizeof(graphic_mode));

	*/

}

///Show the current frame and prepare the next one
void DX11Graphics::Commit(){

	swap_chain_->Present(IsVSync() ? 1 : 0,
						 0);

}

#endif