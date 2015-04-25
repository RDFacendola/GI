#include "dx11graphics.h"

#include <map>

#include <d3d11.h>
#include <dxgi.h>

#include "..\..\include\core.h"
#include "..\..\include\scene.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\resources.h"
#include "..\..\include\renderers.h"
#include "..\..\include\bundles.h"

#include "dx11resources.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

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

	/// \brief DirectX 11 API support.
	const D3D_FEATURE_LEVEL kFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	/// \brief Default video format for the back buffer.
	const DXGI_FORMAT kVideoFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

	/// \brief Convert a dxgi sample desc to an antialiasing mode.
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

	/// \brief Convert a video mode to a dxgi mode.
	DXGI_MODE_DESC VideoModeToDXGIMode(const VideoMode & video_mode){

		DXGI_MODE_DESC dxgi_mode;

		ZeroMemory(&dxgi_mode, sizeof(dxgi_mode));

		dxgi_mode.Width = video_mode.horizontal_resolution;
		dxgi_mode.Height = video_mode.vertical_resolution;
		dxgi_mode.RefreshRate.Denominator = 1000;
		dxgi_mode.RefreshRate.Numerator = static_cast<unsigned int>(video_mode.refresh_rate * dxgi_mode.RefreshRate.Denominator);
		dxgi_mode.Format = kVideoFormat;

		return dxgi_mode;

	}

	/// \brief Convert a dxgi mode to a video mode.
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

	/// \brief Loader class. Maps every resource with their respective loader.
	class Loader{

		using LoaderFunction = unique_ptr<IResource>(*)(ID3D11Device &, const void *);
		using LoaderKey = pair < std::type_index, std::type_index >;
		using LoaderMap = map < LoaderKey, LoaderFunction >;

	public:

		/// \brief Load a resource.
		/// \param type_index Type of the resource to load.
		/// \param load_mode Index of the load mode.
		/// \param device Device used to create the resource.
		/// \param settings Settings used to load the resource.
		/// \return Returns a shared pointer to the loaded resource.
		static unique_ptr<IResource> Load(const std::type_index & resource_type, const std::type_index & bundle_type, ID3D11Device & device, const void * bundle){

			auto key = make_pair(resource_type, bundle_type);

			auto it = loader_map_.find(key);

			return it == loader_map_.end() ?
				unique_ptr<IResource>() :			// Not supported
				it->second(device, bundle);
			
		}

	private:

		/// \brief Load routine dispatcher.
		template <typename TResource, typename TBundle>
		static unique_ptr<IResource> LoadResource(ID3D11Device & device, const void * settings){

			// The resource created is the mapped type of TResource (eg: Texture2D => DX11Texture2D)
			return make_unique<typename ResourceMapping<TResource>::TMapped>(device,
																			 *static_cast<const TBundle*>(settings));

		}

		/// \brief Register the support for a resource type.
		template <typename TResource, typename TBundle>
		static LoaderMap::value_type Register(){

			auto key = make_pair(std::type_index(typeid(TResource)), 
								 std::type_index(typeid(TBundle)));

			auto value = LoadResource < TResource, TBundle >;

			return LoaderMap::value_type(key, value);

		}
			
		static const LoaderMap loader_map_;

	};

	// Add support for new resources HERE!

	const Loader::LoaderMap Loader::loader_map_{ Loader::Register<Texture2D, LoadFromFile>(),
												 Loader::Register<Mesh, BuildFromVertices<VertexFormatNormalTextured>>(),
												 Loader::Register<Material, CompileFromFile>(), 
												 Loader::Register<Material, InstantiateFromMaterial>() };

}

//////////////////////////////////// GRAPHICS ////////////////////////////////////

DX11Graphics & DX11Graphics::GetInstance(){

	static DX11Graphics graphics;

	return graphics;

}

DX11Graphics::DX11Graphics(): Graphics(){

	IDXGIFactory * factory;
	IDXGIAdapter * adapter;
	ID3D11Device * device;

	//DXGI Factory
	THROW_ON_FAIL(CreateDXGIFactory(__uuidof(IDXGIFactory),
								    (void**)(&factory)));

	//DXGI Adapter
	THROW_ON_FAIL(factory->EnumAdapters(kPrimaryOutputIndex,
										&adapter));

	//DXGI Device
	THROW_ON_FAIL(D3D11CreateDevice(kDefaultAdapter,
									D3D_DRIVER_TYPE_HARDWARE,
									0,
									0,
									&kFeatureLevel,
									1,
									D3D11_SDK_VERSION,
									&device,
									nullptr,
									nullptr));

	// Binds the objects to their smart pointers
	
	factory_ = std::move(unique_com(factory));
	adapter_ = std::move(unique_com(adapter));
	device_ = std::move(unique_com(device));
	
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

unique_ptr<Output> DX11Graphics::CreateOutput(Window & window, const VideoMode & video_mode){

	return std::make_unique<DX11Output>(window, video_mode);

}

DX11Resources & DX11Graphics::GetResources(){

	static DX11Resources resources(*device_);

	return resources;

}

unique_ptr<IRenderer> DX11Graphics::CreateRenderer(const type_index & renderer_type, const type_index & renderer_args_type, const void * renderer_args) const{

	return nullptr;

}

//////////////////////////////////// OUTPUT //////////////////////////////////////////

DX11Output::DX11Output(Window & window, const VideoMode & video_mode) :
	window_(window){

	fullscreen_ = false;							// Windowed
	vsync_ = false;									// Disabled by default
	antialiasing_ = AntialiasingMode::NONE;			// Disabled by default.

	video_mode_ = video_mode;

	UpdateSwapChain();

	//Listeners
	on_window_resized_listener_ = window_.OnResized().Subscribe([this](Listener&, Window::OnResizedEventArgs&){

		// Release the old back buffer
		render_target_->ResetBuffers();

		//Resize the swapchain buffer
		swap_chain_->ResizeBuffers(kBuffersCount,
								   0,						//Will fit the client width
								   0,						//Will fit the client height
								   kVideoFormat,
								   0);

		DXGI_SWAP_CHAIN_DESC desc;

		swap_chain_->GetDesc(&desc);

		video_mode_ = DXGIModeToVideoMode(desc.BufferDesc);

		UpdateBackbuffer();
		
	});

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

		UpdateSwapChain();

	}

}

void DX11Output::UpdateSwapChain(){

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

	IDXGISwapChain * swap_chain;
		
	auto& graphics = DX11Graphics::GetInstance();

	auto& factory = graphics.GetFactory();
	auto& device = graphics.GetDevice();

	THROW_ON_FAIL(factory.CreateSwapChain(&device,
										  &dxgi_desc,
										  &swap_chain));

	swap_chain_ = std::move(unique_com(swap_chain));

	UpdateBackbuffer();
	
	// Restore the fullscreen state
	SetFullscreen(fullscreen_);

}

void DX11Output::UpdateBackbuffer(){

	ID3D11Texture2D * back_buffer;

	swap_chain_->GetBuffer(0,
						   __uuidof(back_buffer),
						   reinterpret_cast<void**>(&back_buffer));
	
	if (!render_target_){

		render_target_ = std::make_shared<DX11RenderTarget>(*back_buffer);

	}
	else{

		render_target_->SetBuffers({ back_buffer });

	}
	
}

/////////////////////////////////// RESOURCES ///////////////////////////////////////////

unique_ptr<IResource> DX11Resources::Load(const type_index & resource_type, const type_index & bundle_type, const void * bundle) const{

	return Loader::Load(resource_type, bundle_type, device_, bundle);

}