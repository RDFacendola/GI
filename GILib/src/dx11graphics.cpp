#ifdef _WIN32

#include "dx11graphics.h"

#include <d3d11.h>
#include <dxgi.h>

#include "window.h"
#include "exceptions.h"

using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace std;

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

DX11Graphics::DX11Graphics(Window & window, ID3D11Device & device, IDXGIFactory & factory) :
	window_(window),
	device_(device),
	factory_(factory),
	swap_chain_(nullptr){

	///VSync disabled by default
	SetVSync(false);

	//Create the swapchain with default parameters
	CreateSwapChain(GetDefaultSwapchainMode());

	//Listeners
	on_window_resized_listener_ = window_.OnResized().AddListener([this](Window &, unsigned int, unsigned int){

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