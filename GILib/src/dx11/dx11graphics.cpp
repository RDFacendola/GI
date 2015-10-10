#include "dx11/dx11graphics.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <d3d11.h>
#include <dxgi.h>
#include <map>

#ifdef _DEBUG

#pragma comment(lib,"dxguid.lib")

#include <Initguid.h>
#include <dxgidebug.h>

#endif

#include "core.h"
#include "scene.h"
#include "exceptions.h"
#include "resources.h"
#include "instance_builder.h"
#include "deferred_renderer.h"
#include "scope_guard.h"

#include "dx11/dx11render_target.h"
#include "dx11/dx11renderer.h"
#include "dx11/dx11deferred_renderer.h"
#include "dx11/dx11mesh.h"
#include "dx11/dx11sampler.h"
#include "dx11/dx11gpgpu.h"

#undef max
#undef min

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

//////////////////////////////////// ANONYMOUS ///////////////////////////////////

namespace{

	/// \brief Index of the primary output.
	const unsigned int kPrimaryOutputIndex = 0;

	/// \brief Index of the default video card.
	const unsigned int kDefaultAdapterIndex = 0;

	/// \brief Pointer to the default video card.
	IDXGIAdapter * const kDefaultAdapter = nullptr;

	/// \brief Number of buffers used by the swapchain.
	const unsigned int kBuffersCount = 3;

	/// \brief Minimum resolution allowed, in pixels.
	const unsigned int kMinimumResolution = 1024 * 768;

	/// \brief This value replaces the backbuffer dimensions if those become too small after a window resize.
	const unsigned int kMinimumBackbufferDimension = 8;
	
	/// \brief DirectX 11 API support.
	const D3D_FEATURE_LEVEL kFeatureLevel = D3D_FEATURE_LEVEL_11_0;

#ifndef BGRA_SUPPORT
	
	/// \brief Default video format for the back buffer.
	const DXGI_FORMAT kVideoFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

#else
	
	/// \brief Default video format for the back buffer.
	const DXGI_FORMAT kVideoFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

#endif

	/// \brief Convert a DXGI sample desc to an antialiasing mode.
	AntialiasingMode SampleDescToAntialiasingMode(const DXGI_SAMPLE_DESC & sample_desc){

		if (sample_desc.Count == 1 && sample_desc.Quality == 0)		return AntialiasingMode::NONE;
		if (sample_desc.Count == 2 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_2X;
		if (sample_desc.Count == 4 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_4X;
		if (sample_desc.Count == 8 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_8X;
		if (sample_desc.Count == 16 && sample_desc.Quality == 0)	return AntialiasingMode::MSAA_16X;

		return AntialiasingMode::NONE;

	}

	/// \brief Convert an antialiasing mode to a sample desc.
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

	/// \brief Convert a video mode to a DXGI mode.
	DXGI_MODE_DESC VideoModeToDXGIMode(const VideoMode& video_mode){

		DXGI_MODE_DESC dxgi_mode;

		ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));

		dxgi_mode.Width = video_mode.horizontal_resolution;
		dxgi_mode.Height = video_mode.vertical_resolution;
		dxgi_mode.RefreshRate.Denominator = 1000;
		dxgi_mode.RefreshRate.Numerator = static_cast<unsigned int>(video_mode.refresh_rate * dxgi_mode.RefreshRate.Denominator);
		dxgi_mode.Format = kVideoFormat;

		return dxgi_mode;

	}

	/// \brief Convert a DXGI mode to a video mode.
	VideoMode DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi_mode){

		VideoMode video_mode;

		video_mode.horizontal_resolution = dxgi_mode.Width;
		video_mode.vertical_resolution = dxgi_mode.Height;
		video_mode.refresh_rate = static_cast<unsigned int>(std::round(static_cast<float>(dxgi_mode.RefreshRate.Numerator) / dxgi_mode.RefreshRate.Denominator));

		return video_mode;

	}

	/// \brief Enumerate the supported DXGI video modes
	vector<DXGI_MODE_DESC> EnumerateDXGIModes(IDXGIAdapter & adapter){

		IDXGIOutput * adapter_output = nullptr;
		unsigned int output_mode_count;

		THROW_ON_FAIL(adapter.EnumOutputs(kPrimaryOutputIndex,
										  &adapter_output));

		COM_GUARD(adapter_output);

		THROW_ON_FAIL(adapter_output->GetDisplayModeList(kVideoFormat,
														 0,
														 &output_mode_count,
														 nullptr));

		vector<DXGI_MODE_DESC> dxgi_modes(output_mode_count);

		THROW_ON_FAIL(adapter_output->GetDisplayModeList(kVideoFormat,
														 0,
														 &output_mode_count,
														 &dxgi_modes[0]));

		return dxgi_modes;

	}

	/// \brief Enumerate the supported video modes. Filters by resolution and refresh rate
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

	/// \brief Enumerate the supported antialiasing modes
	vector<AntialiasingMode> EnumerateAntialiasingModes(ID3D11Device & device){

		vector<AntialiasingMode> antialiasing_modes;

		DXGI_SAMPLE_DESC sample;

		unsigned int sample_quality_max;

		AntialiasingMode antialiasing_mode;

		for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++sample_count){

			//If the maximum supported quality is 0 the mode is not supported 	
			THROW_ON_FAIL(device.CheckMultisampleQualityLevels(kVideoFormat,
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

//////////////////////////////////// OUTPUT //////////////////////////////////////////

DX11Output::DX11Output(windows::Window & window, const VideoMode& video_mode) :
	window_(window){

	fullscreen_ = false;							// Windowed
	vsync_ = false;									// Disabled by default
	antialiasing_ = AntialiasingMode::NONE;			// Disabled by default.

	video_mode_ = video_mode;

	CreateSwapChain();

	// Listeners
	on_window_resized_listener_ = window_.OnResized().Subscribe([this](Listener&, Window::OnResizedEventArgs& args){

		// Store the new dimensions of the buffer

		video_mode_.horizontal_resolution = std::max(args.width, kMinimumBackbufferDimension);
		video_mode_.vertical_resolution = std::max(args.height, kMinimumBackbufferDimension);

		// Release the old backbuffer and its render target

		back_buffer_ = nullptr;								
		render_target_ = nullptr;
		
		//Resize the swapchain buffer

		swap_chain_->ResizeBuffers(kBuffersCount,
								   video_mode_.horizontal_resolution,
								   video_mode_.vertical_resolution,
								   kVideoFormat,
								   0);

		UpdateBackbuffer();
		
	});

	// Scaler
	scaler_ = make_unique<fx::DX11FxScaler>();

}

DX11Output::~DX11Output(){

	//Return to windowed mode (otherwise the screen will hang)
	SetFullscreen(false);

}

void DX11Output::SetVideoMode(const VideoMode & video_mode){

	video_mode_ = video_mode;

	auto dxgi_mode = VideoModeToDXGIMode(video_mode);

	swap_chain_->ResizeTarget(&dxgi_mode);	//Resizing the target will trigger the resizing of the backbuffer as well.

}

void DX11Output::SetFullscreen(bool fullscreen){

	fullscreen_ = fullscreen;

	swap_chain_->SetFullscreenState(fullscreen,
									nullptr);

}

void DX11Output::SetAntialiasing(AntialiasingMode antialiasing){

	if (antialiasing_ != antialiasing){

		antialiasing_ = antialiasing;

		CreateSwapChain();

	}

}

void DX11Output::CreateSwapChain(){

	// Description from video mode and antialiasing mode
	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));

	dxgi_desc.BufferCount = kBuffersCount;
	dxgi_desc.OutputWindow = window_.GetHandle();
	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgi_desc.Windowed = true;
	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;

	dxgi_desc.BufferDesc = VideoModeToDXGIMode(video_mode_);
	dxgi_desc.SampleDesc = AntialiasingModeToSampleDesc(antialiasing_);
	
	// Create the actual swap chain

	IDXGISwapChain* swap_chain;
		
	auto& graphics = DX11Graphics::GetInstance();

	auto factory = graphics.GetFactory();
	auto device = graphics.GetDevice();

	THROW_ON_FAIL(factory->CreateSwapChain(device.Get(),
										   &dxgi_desc,
										   &swap_chain));

	swap_chain_ << &swap_chain;

	UpdateBackbuffer();
	
	// Restore the fullscreen state
	SetFullscreen(fullscreen_);

}

void DX11Output::UpdateBackbuffer(){

	ID3D11Texture2D * back_buffer;

	swap_chain_->GetBuffer(0,
						   __uuidof(back_buffer),
						   reinterpret_cast<void**>(&back_buffer));
	
	back_buffer_ << &back_buffer;

	// Associate a new render target

	auto device = DX11Graphics::GetInstance().GetDevice();

	ID3D11RenderTargetView* rtv;

	THROW_ON_FAIL(device->CreateRenderTargetView(back_buffer_.Get(),
												 nullptr,
												 &rtv));

	render_target_ = new DX11RenderTarget(COMMove(&rtv));

}

void DX11Output::Display(ObjectPtr<ITexture2D>& image){

	// Copy and scale the content of the image to the actual backbuffer

	scaler_->Copy(image,
				  render_target_);
	
	// Flip the burgers... ehm buffers!

	swap_chain_->Present(IsVSync() ? 1 : 0,
						 0);

}

/////////////////////////////////// RESOURCES ///////////////////////////////////////////

DX11Resources& DX11Resources::GetInstance(){

	static DX11Resources resource_manager;

	return resource_manager;

}

DX11Resources::DX11Resources(){}

ObjectPtr<IResource> DX11Resources::Load(const type_index& resource_type, const type_index& args_type, const void* args) const{

	return static_cast<IResource*>(InstanceBuilder::Build(resource_type, 
														  args_type, 
														  args));
	
}

//////////////////////////////////// GRAPHICS ////////////////////////////////////

DX11Graphics & DX11Graphics::GetInstance(){

	static DX11Graphics graphics;

	return graphics;

}

DX11Graphics::DX11Graphics(): Graphics(){

	IDXGIFactory* factory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	auto&& guard = make_scope_guard([&factory, &adapter, &device, &context](){

		if (context)	context->Release();
		if (factory)	factory->Release();
		if (adapter)	adapter->Release();
		if (device)		device->Release();

	});

	//DXGI Factory
	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory),
								    (void**)(&factory)));

	//DXGI Adapter
	THROW_ON_FAIL(factory->EnumAdapters(kPrimaryOutputIndex,
										&adapter));
	
#ifdef _DEBUG

	unsigned int device_flags = D3D11_CREATE_DEVICE_DEBUG;

#else

	unsigned int device_flags = 0;
	
#endif

// 	if (kVideoFormat == DXGI_FORMAT_B8G8R8A8_UNORM) {
// 
// 		device_flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
// 
// 	}

	//DXGI Device
	THROW_ON_FAIL(D3D11CreateDevice(kDefaultAdapter,
									D3D_DRIVER_TYPE_HARDWARE,
									0,
									device_flags,
									&kFeatureLevel,
									1,
									D3D11_SDK_VERSION,
									&device,
									nullptr,
									nullptr));

	// Context
	device->GetImmediateContext(&context);

	// Move the ownership
	
	factory_ << &factory;
	adapter_ << &adapter;
	device_ << &device;
	immediate_context_ << &context;
	
	// Cleanup

	guard.Dismiss();

}

DX11Graphics::~DX11Graphics(){

	FlushAll();			// Clear the context and flush remaining commands.

	immediate_context_ = nullptr;
	factory_ = nullptr;

	// Only the device is allowed here!
	DebugReport();

}

void DX11Graphics::FlushAll() {

	// Remove any pending reference

	immediate_context_->ClearState();

	// Flush any pending command

	D3D11_QUERY_DESC query_desc;
	ID3D11Query* query;

	query_desc.Query = D3D11_QUERY_EVENT;
	query_desc.MiscFlags = 0;

	// Flush the command queue and synchronously wait until it becomes empty

	if (SUCCEEDED(device_->CreateQuery(&query_desc,
									   &query))) {

		immediate_context_->Flush();

		immediate_context_->End(query);

		while (!SUCCEEDED(immediate_context_->GetData(query, nullptr, 0, 0))) {}	// Spin, spin!

		query->Release();

	}

}

void DX11Graphics::DebugReport() {

#ifdef _DEBUG

	ID3D11Debug* d3d_debug;

	if (SUCCEEDED(device_->QueryInterface(__uuidof(ID3D11Debug),
										  reinterpret_cast<void**>(&d3d_debug)))) {

		device_ = nullptr;

		if (FAILED(d3d_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY))) {

			OutputDebugString(L"Unable to report DirectX live objects");

		}
			
	}

	DebugBreak();		// Check the output log!

#endif

}

AdapterProfile DX11Graphics::GetAdapterProfile() const{

	AdapterProfile adapter_profile;
	DXGI_ADAPTER_DESC adapter_desc;

	THROW_ON_FAIL(adapter_->GetDesc(&adapter_desc));

	adapter_profile.name = wstring(adapter_desc.Description);
	adapter_profile.dedicated_memory = adapter_desc.DedicatedVideoMemory;
	adapter_profile.shared_memory = adapter_desc.SharedSystemMemory;
	adapter_profile.video_modes = EnumerateVideoModes(*adapter_);
	adapter_profile.antialiasing_modes = EnumerateAntialiasingModes(*device_);

	adapter_profile.max_anisotropy = D3D11_MAX_MAXANISOTROPY;
	adapter_profile.max_mips = D3D11_REQ_MIP_LEVELS;

	return adapter_profile;

}

unique_ptr<IOutput> DX11Graphics::CreateOutput(gi_lib::Window& window, const VideoMode& video_mode){

	return std::make_unique<DX11Output>(static_cast<windows::Window&>(window), 
										video_mode);

}

DX11Resources& DX11Graphics::GetResources(){

	return DX11Resources::GetInstance();

}

IRenderer* DX11Graphics::CreateRenderer(const type_index& renderer_type, Scene& scene) const{

	// The construction args are the same for every renderer
	RendererConstructionArgs construction_args(scene);

	return static_cast<IRenderer*>(InstanceBuilder::Build(renderer_type,
														  type_index(typeid(RendererConstructionArgs)),
														  &construction_args));
	
}