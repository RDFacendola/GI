#include "dx11graphics.h"

#include <algorithm>

#include "exceptions.h"
#include "release_guard.h"

DX11Graphics::DX11Graphics() :
	device_(nullptr),
	swap_chain_(nullptr),
	backbuffer_view_(nullptr)
{}

///Create the graphic interface. Throws on failure
void DX11Graphics::CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode)
{

	auto feature_levels = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));

	dxgi_desc.BufferCount = 3;	//Triple buffering only
	dxgi_desc.OutputWindow = window_handle;
	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgi_desc.Windowed = graphic_mode.windowed;
	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	
	//Back buffer
	auto video_mode = GetVideoModeOrDie(graphic_mode);

	dxgi_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//True color only
	dxgi_desc.BufferDesc.Width = video_mode.resolution.width;
	dxgi_desc.BufferDesc.Height = video_mode.resolution.height;
	dxgi_desc.BufferDesc.RefreshRate.Numerator = video_mode.refresh_rate.numerator;
	dxgi_desc.BufferDesc.RefreshRate.Denominator = video_mode.refresh_rate.denominator;

	//Antialiasing
	auto multisample = GetMultisampleOrDie(graphic_mode.antialiasing);

	dxgi_desc.SampleDesc.Count = multisample.count;
	dxgi_desc.SampleDesc.Quality = multisample.quality;

	//Create the device and the swapchain
	THROW_ON_FAIL(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &feature_levels, 1, D3D11_SDK_VERSION, &dxgi_desc, &swap_chain_, &device_, nullptr, &immediate_context_));

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

VIDEO_MODE DX11Graphics::GetVideoModeOrDie(const GRAPHIC_MODE & graphic_mode)
{

	ADAPTER_PROFILE profile;

	SystemProfiler::GetAdapterProfileOrDie(profile);

	std::vector<const VIDEO_MODE> video_modes(profile.supported_video_modes.size());

	// select modes with matching resolution
	auto it = std::copy_if(profile.supported_video_modes.begin(),
						   profile.supported_video_modes.end(),
						   video_modes.begin(),
						   [=](const VIDEO_MODE vm)
						   {

								return vm.resolution.width == graphic_mode.horizontal_resolution &&
									   vm.resolution.height == graphic_mode.vertical_resolution;

						   });

	video_modes.resize(std::distance(video_modes.begin(),
								     it));  // shrink container to new size

	// pick the mode with the highest refresh rate
	return *std::max_element(video_modes.begin(),
							 video_modes.end(),
							 [](VIDEO_MODE first, VIDEO_MODE second)
							 {

								 return first.refresh_rate.GetHz() < second.refresh_rate.GetHz();

							 });

}

MULTISAMPLE DX11Graphics::GetMultisampleOrDie(const ANTIALIASING_MODE & antialiasing_mode)
{

	MULTISAMPLE multisample;

	switch (antialiasing_mode)
	{

	case ANTIALIASING_MODE::NONE:

		multisample.count = 1;
		multisample.quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_2X:

		multisample.count = 2;
		multisample.quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_4X:

		multisample.count = 4;
		multisample.quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_8X:

		multisample.count = 8;
		multisample.quality = 0;
		break;

	case ANTIALIASING_MODE::MSAA_16X:

		multisample.count = 16;
		multisample.quality = 0;
		break;

	default:

		throw RuntimeException(L"Invalid antialiasing mode");

	}

	return multisample;

}