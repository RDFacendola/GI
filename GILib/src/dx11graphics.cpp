//#include "dx11graphics.h"	
//
//#include <algorithm>
//#include <memory>
//
//#include "exceptions.h"
//#include "raii.h"
//
//const unsigned int kPrimaryDisplayIndex = 0;
//
//const unsigned int kDefaultAdapterIndex = 0;
//
//IDXGIAdapter * const kDefaultAdapter = nullptr;
//
//const unsigned int kMinimumResolution = 1024 * 768;
//
//const DXGI_FORMAT kGraphicFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//const D3D_FEATURE_LEVEL kFeatureLevel = D3D_FEATURE_LEVEL_11_0;
//
////DX11Utils
//
///// Convert a multisample structure to an antialiasing mode
//ANTIALIASING_MODE DX11Utils::SampleToAntialiasing(const DXGI_SAMPLE_DESC & sample){
//
//	if (sample.Count == 1 && sample.Quality == 0)		return ANTIALIASING_MODE::NONE;
//	if (sample.Count == 2 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_2X;
//	if (sample.Count == 4 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_4X;
//	if (sample.Count == 8 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_8X;
//	if (sample.Count == 16 && sample.Quality == 0)		return ANTIALIASING_MODE::MSAA_16X;
//
//	return ANTIALIASING_MODE::NONE;
//
//}
//
///// Convert an antialiasing mode to a multisample structure
//DXGI_SAMPLE_DESC DX11Utils::AntialiasingToSample(const ANTIALIASING_MODE & antialiasing){
//
//	DXGI_SAMPLE_DESC sample;
//
//	switch (antialiasing)
//	{
//
//	case ANTIALIASING_MODE::NONE:
//
//		sample.Count = 1;
//		sample.Quality = 0;
//		break;
//
//	case ANTIALIASING_MODE::MSAA_2X:
//
//		sample.Count = 2;
//		sample.Quality = 0;
//		break;
//
//	case ANTIALIASING_MODE::MSAA_4X:
//
//		sample.Count = 4;
//		sample.Quality = 0;
//		break;
//
//	case ANTIALIASING_MODE::MSAA_8X:
//
//		sample.Count = 8;
//		sample.Quality = 0;
//		break;
//
//	case ANTIALIASING_MODE::MSAA_16X:
//
//		sample.Count = 16;
//		sample.Quality = 0;
//		break;
//
//	}
//
//	return sample;
//
//}
//
///// Convert a video mode to a dxgi mode
//DXGI_MODE_DESC DX11Utils::VideoModeToDXGIMode(const VIDEO_MODE & video){
//
//	DXGI_MODE_DESC dxgi_mode;
//
//	ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));
//
//	dxgi_mode.Width = video.horizontal_resolution;
//	dxgi_mode.Height = video.vertical_resolution;
//	dxgi_mode.RefreshRate.Denominator = 1000;
//	dxgi_mode.RefreshRate.Numerator = static_cast<unsigned int>(video.refresh_rate_Hz * dxgi_mode.RefreshRate.Denominator);
//	dxgi_mode.Format = kGraphicFormat;
//
//	return dxgi_mode;
//
//}
//
///// Convert a dxgi mode to a video mode
//VIDEO_MODE DX11Utils::DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi){
//
//	VIDEO_MODE video_mode;
//
//	video_mode.horizontal_resolution = dxgi.Width;
//	video_mode.vertical_resolution = dxgi.Height;
//	video_mode.refresh_rate_Hz = static_cast<unsigned int>(std::round(static_cast<float>(dxgi.RefreshRate.Numerator) / dxgi.RefreshRate.Denominator));
//
//	return video_mode;
//
//}
//
////DX11Factory
//
/////Create a new factory
//DX11Factory::DX11Factory():
//	device_(nullptr){
//
//	//DXGI Factory
//	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory),
//									(void**)(&factory_)));
//
//	//DXGI Adapter
//	THROW_ON_FAIL(GetFactory().EnumAdapters(kPrimaryDisplayIndex,
//											&adapter_));
//	//DXGI Device
//	THROW_ON_FAIL(D3D11CreateDevice(kDefaultAdapter,
//									D3D_DRIVER_TYPE_HARDWARE,
//									0,
//									0,
//									&kFeatureLevel,
//									1,
//									D3D11_SDK_VERSION,
//									&device_,
//									nullptr,
//									nullptr));
//
//}
//
/////Destroy the factory
//DX11Factory::~DX11Factory(){
//
//	device_->Release();
//	adapter_->Release();
//	factory_->Release();
//
//}
//
/////Get the capabilities for this API
//ADAPTER_PROFILE DX11Factory::GetProfile() const{
//
//	ADAPTER_PROFILE adapter_profile;
//	DXGI_ADAPTER_DESC adapter_desc;
//
//	auto & adapter = GetAdapter();
//
//	THROW_ON_FAIL(adapter.GetDesc(&adapter_desc));
//
//	adapter_profile.dedicated_memory = adapter_desc.DedicatedVideoMemory;
//	adapter_profile.shared_memory = adapter_desc.SharedSystemMemory;
//	adapter_profile.adapter_name = wstring(adapter_desc.Description);
//	adapter_profile.supported_video = EnumerateVideoModes();
//	adapter_profile.supported_antialiasing = EnumerateAntialiasingModes();
//
//	//
//	return adapter_profile;
//
//}
//
/////Create the graphic object
//BaseGraphics * DX11Factory::Create(Window & window){
//
//	return new DX11Graphics(window,
//							*this);
//	
//}
//
///// Enumerate the supported antialiasing modes
//vector<const VIDEO_MODE> DX11Factory::EnumerateVideoModes() const{
//	
//	auto dxgi_modes = EnumerateDXGIModes();
//
//	//Remove the modes that do not satisfy the minimum requirements
//	dxgi_modes.erase(std::remove_if(dxgi_modes.begin(), 
//								    dxgi_modes.end(), 
//								    [](const DXGI_MODE_DESC & value){ 
//		
//										return value.Width * value.Height < kMinimumResolution; 
//	
//									}), 
//					 dxgi_modes.end());
//
//	//Sorts the modes by width, height and refresh rate
//	std::sort(dxgi_modes.begin(), 
//			  dxgi_modes.end(),
//			  [](const DXGI_MODE_DESC & first, const DXGI_MODE_DESC & second){
//
//					return first.Width < second.Width ||
//						   first.Width == second.Width &&
//						   (first.Height < second.Height ||
//						    first.Height == second.Height &&
//							first.RefreshRate.Numerator * second.RefreshRate.Denominator > second.RefreshRate.Numerator * first.RefreshRate.Denominator);
//						   
//
//			  });
//
//	//Keeps the highest refresh rate for each resolution combination and discards everything else
//	dxgi_modes.erase(std::unique(dxgi_modes.begin(), 
//							     dxgi_modes.end(),
//							     [](const DXGI_MODE_DESC & first, const DXGI_MODE_DESC & second){
//		
//									return first.Width == second.Width &&
//									first.Height == second.Height;
//
//								 }), 
//					dxgi_modes.end());
//
//	//Map DXGI_MODE_DESC to VIDEO_MODE
//	vector<const VIDEO_MODE> video_modes(dxgi_modes.size());
//
//	std::transform(dxgi_modes.begin(),
//				   dxgi_modes.end(),
//				   video_modes.begin(),
//				   [this](const DXGI_MODE_DESC & mode)
//				   {
//	
//						return DX11Utils::DXGIModeToVideoMode(mode);
//	
//				   });
//
//	return video_modes;
//
//}
//
///// Enumerate the supported video modes. Filters by resolution and refresh rate
//vector<const DXGI_MODE_DESC> DX11Factory::EnumerateDXGIModes() const{
//
//	IDXGIOutput * adapter_output = nullptr;
//	unsigned int output_mode_count;
//
//	THROW_ON_FAIL(GetAdapter().EnumOutputs(kPrimaryDisplayIndex,
//										   &adapter_output));
//
//	//Release the output when the function returns or throws
//	auto guard = ScopeGuard([adapter_output](){ adapter_output->Release(); });
//
//	THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
//													 0,
//													 &output_mode_count,
//													 nullptr));
//
//	vector<const DXGI_MODE_DESC> dxgi_modes(output_mode_count);
//
//	THROW_ON_FAIL(adapter_output->GetDisplayModeList(kGraphicFormat,
//													  0,
//													  &output_mode_count,
//													  &dxgi_modes[0]));
//
//	return dxgi_modes;
//
//}
//
///// Enumerate the supported DXGI video modes
//vector<const ANTIALIASING_MODE> DX11Factory::EnumerateAntialiasingModes() const{
//
//	vector<const ANTIALIASING_MODE> antialiasing_modes;
//
//	auto & device = GetDevice();
//
//	DXGI_SAMPLE_DESC sample;
//
//	unsigned int sample_quality_max;
//
//	//Samples must be multiple of 2
//	for (unsigned int sample_count = 1; sample_count < D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sample_count *= 2){
//
//		//If the maximum supported quality is 0 the mode is not supported 	
//		THROW_ON_FAIL(device.CheckMultisampleQualityLevels(kGraphicFormat,
//														   sample_count,
//														   &sample_quality_max));
//
//		if (sample_quality_max > 0){
//			
//			//Add the lowest quality for that amount of samples
//			sample.Count = sample_count;
//			sample.Quality = 0;
//
//			antialiasing_modes.push_back(DX11Utils::SampleToAntialiasing(sample));
//
//			//Increase the quality exponentially through the maximum value
//			for (unsigned int current_quality = 1; current_quality < sample_quality_max; current_quality *= 2){
//
//				sample.Quality = current_quality;
//
//				antialiasing_modes.push_back(DX11Utils::SampleToAntialiasing(sample));
//
//			}
//
//		}
//
//	}
//
//	//Unknown modes are "NONE". Remove the doubles
//	antialiasing_modes.erase(std::remove(antialiasing_modes.begin(),
//										 antialiasing_modes.end(),
//										 ANTIALIASING_MODE::NONE),
//							 antialiasing_modes.end());
//
//	antialiasing_modes.insert(antialiasing_modes.begin(),
//							  ANTIALIASING_MODE::NONE);
//
//	return antialiasing_modes;
//
//}
//
////DX11Graphics
//
/////Create a new graphics object
//DX11Graphics::DX11Graphics(Window & window, DX11Factory & factory) :
//	window_(window),
//	device_(factory.GetDevice()),
//	factory_(factory){
//
//	///VSync disabled by default
//	EnableVSync(false);
//
//	//Create the swapchain with default parameters
//	CreateSwapChain(GetDefaultSwapchainMode());
//
//	//Listener used to resize the swapchain during the window resize
//	on_window_message_listener_ = new Window::TMessageEvent::TListener([this](Window & window, unsigned int message_id, WPARAM wparam, LPARAM lparam){
//
//		if (message_id == WM_SIZE){
//
//			//Resize the swapchain buffer
//			swap_chain_->ResizeBuffers(3, 0, 0, kGraphicFormat, 0);
//			
//		}
//
//	});
//
//	window_.OnMessage() << *on_window_message_listener_;
//
//}
//
/////Destroy the graphic object
//DX11Graphics::~DX11Graphics(){
//
//	//Return to windowed mode
//	EnableFullscreen(false);
//
//	swap_chain_->Release();
//
//}
//
/////Get the resources' loader
//IResources & DX11Graphics::GetResources(){
//
//	return *resources_;
//
//}
//
/////Get the renderer
//IRenderer & DX11Graphics::GetRenderer(){
//
//	return *renderer_;
//
//}
//
/////Show the current frame and prepare the next one
//void DX11Graphics::NextFrame(){
//
//	swap_chain_->Present(IsVSyncEnabled() ? 1 : 0, 
//						 0);
//
//}
//
/////Set the video mode
//void DX11Graphics::SetVideo(const VIDEO_MODE & video){
//
//	BaseGraphics::SetVideo(video);
//
//	//Resize the target window (this will cause the resize of the backbuffer)
//
//	DXGI_MODE_DESC dxgi_mode;
//
//	ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));
//
//	dxgi_mode.Format = kGraphicFormat;
//	dxgi_mode.Width = video.horizontal_resolution;
//	dxgi_mode.Height = video.vertical_resolution;
//	dxgi_mode.RefreshRate.Denominator = 1000;
//	dxgi_mode.RefreshRate.Numerator = video.refresh_rate_Hz * dxgi_mode.RefreshRate.Denominator;
//	
//	swap_chain_->ResizeTarget(&dxgi_mode);
//
//}
//
/////Set the antialising mode
//void DX11Graphics::SetAntialising(const ANTIALIASING_MODE & antialiasing){
//
//	BaseGraphics::SetAntialising(antialiasing);
//
//	//Create the swapchain again (keeps everything except the antialiasing)
//	DXGI_SWAP_CHAIN_DESC dxgi_desc;
//
//	swap_chain_->GetDesc(&dxgi_desc);
//
//	dxgi_desc.SampleDesc = DX11Utils::AntialiasingToSample(antialiasing);
//
//	CreateSwapChain(dxgi_desc);
//
//}
//
/////Enable or disable the fullscreen state
//void DX11Graphics::EnableFullscreen(bool enable){
//
//	BaseGraphics::EnableFullscreen(enable);
//
//	swap_chain_->SetFullscreenState(enable, 
//									nullptr);
//
//}
//
/////Return the default DXGI mode for the swap chain
//DXGI_SWAP_CHAIN_DESC DX11Graphics::GetDefaultSwapchainMode() const{
//
//	//Create the swapchain with default settings
//	DXGI_SWAP_CHAIN_DESC dxgi_desc;
//
//	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));
//
//	dxgi_desc.BufferCount = 3;	//Triple buffering only
//	dxgi_desc.OutputWindow = window_.GetWindowHandle();
//	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//	dxgi_desc.Windowed = true;
//	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//
//	//Back buffer and antialiasing - Default
//	dxgi_desc.BufferDesc.Format = kGraphicFormat;
//	dxgi_desc.SampleDesc.Count = 1;
//
//	return dxgi_desc;
//
//}
//
/////Create a new swapchain given its description
//void DX11Graphics::CreateSwapChain(DXGI_SWAP_CHAIN_DESC & desc){
//
//	//TODO: Release the outstanding references to the backbuffer in the context
//
//	THROW_ON_FAIL(factory_.GetFactory().CreateSwapChain(&device_,
//														&desc,
//														&swap_chain_));
//	//TODO: Get the references to the backbuffer
//	/*
//
//	//Save the backbuffer view
//
//	ID3D11Texture2D * backbuffer = nullptr;
//
//	ReleaseGuard<ID3D11Texture2D> guard(backbuffer);
//
//	THROW_ON_FAIL(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)));
//
//	THROW_ON_FAIL(device_->CreateRenderTargetView(backbuffer, nullptr, &backbuffer_view_));
//
//	memcpy_s(&graphic_mode_, sizeof(graphic_mode_), &graphic_mode, sizeof(graphic_mode));
//
//	*/
//
//}