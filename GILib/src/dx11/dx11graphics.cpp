#include "dx11/dx11graphics.h"

#include <unordered_map>

#include <d3d11.h>
#include <dxgi.h>

#include "core.h"
#include "exceptions.h"

#include "texture.h"
#include "dx11/dx11texture.h"

#include "dx11/dx11shared.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;

//////////////////////////////////// ANONYMOUS ///////////////////////////////////

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

	/// DirectX 11 API support.
	const D3D_FEATURE_LEVEL kFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	/// Enumerate the supported DXGI video modes
	vector<DXGI_MODE_DESC> EnumerateDXGIModes(IDXGIAdapter & adapter){

		IDXGIOutput * adapter_output = nullptr;
		unsigned int output_mode_count;

		THROW_ON_FAIL(adapter.EnumOutputs(kPrimaryOutputIndex,
			&adapter_output));

		//Release the output when the function returns or throws
		auto guard = unique_ptr<IDXGIOutput, COMDeleter>(adapter_output);

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

	template <typename TResource>
	unique_ptr<Resource> LoadResource(ID3D11Device &, const wstring &, const void * extras);

	template <typename Texture2D>
	unique_ptr<Resource> LoadResource(ID3D11Device & device, const wstring & path, const void *) {

		return make_unique<DX11Texture2D>(device, path);

	};

	// Loader class. Maps every resource with their respective loader.
	class Loader{

		using LoaderFunction = unique_ptr<Resource>(*)(ID3D11Device &, const wstring &, const void * extras);
		using LoaderMap = unordered_map < std::type_index, LoaderFunction >;

	public:

		static Loader GetInstance(){

			static Loader loader;

			return loader;

		}

		/// \brief Load a resource.
		/// \param type_index Type of the resource to load.
		/// \param device Device used to create the resource.
		/// \param path Path of the resource.
		/// \return Returns a shared pointer to the loaded resource
		unique_ptr<Resource> Load(const std::type_index & type_index, ID3D11Device & device, const wstring & path, const void * extras){

			auto it = loader_map_.find(type_index);

			if (it == loader_map_.end()){

				return unique_ptr<Resource>();	// Empty pointer

			}

			return it->second(device, path, extras);

		}

	private:

		Loader(){

			loader_map_.insert(MakeSupport<Texture2D>());

		}

		template <typename TResource>
		LoaderMap::value_type MakeSupport(){

			// Declare the support for the resources...

			return LoaderMap::value_type(type_index(typeid(TResource)), LoadResource<TResource>);

		}

		LoaderMap loader_map_;

	};

}

//////////////////////////////////// GRAPHICS ////////////////////////////////////

DX11Graphics & DX11Graphics::GetInstance(){

	static DX11Graphics graphics;

	return graphics;

}

DX11Graphics::DX11Graphics(){

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
	
	factory_.reset(factory);
	adapter_.reset(adapter);
	device_.reset(device);
	
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

unique_ptr<Output> DX11Graphics::CreateOutput(Window & window){

	return std::make_unique<DX11Output>(window, *device_, *factory_);

}

Manager & DX11Graphics::GetManager(){

	static DX11Manager resources(*device_);

	return resources;

}

//////////////////////////////////// OUTPUT //////////////////////////////////////////

DX11Output::DX11Output(Window & window, ID3D11Device & device, IDXGIFactory & factory) :
	window_(window),
	device_(device),
	factory_(factory){

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
																							   kVideoFormat,
																							   0);

																 });

}

///Destroy the graphic object
DX11Output::~DX11Output(){

	//Return to windowed mode (otherwise the screen will hang)
	SetFullscreen(false);

}

void DX11Output::SetVideoMode(const VideoMode & video_mode){

	video_mode_ = video_mode;

	auto dxgi_mode = VideoModeToDXGIMode(video_mode);

	swap_chain_->ResizeTarget(&dxgi_mode);	//Resizing the target will trigger the resizing of the backbuffer as well.

}

void DX11Output::SetAntialisingMode(const AntialiasingMode & antialiasing_mode){

	antialiasing_mode_ = antialiasing_mode;

	//Create the swapchain again (keeps everything except the antialiasing)
	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	swap_chain_->GetDesc(&dxgi_desc);

	dxgi_desc.SampleDesc = AntialiasingModeToSampleDesc(antialiasing_mode);

	CreateSwapChain(dxgi_desc);

}

void DX11Output::SetFullscreen(bool fullscreen){

	fullscreen_ = fullscreen;

	swap_chain_->SetFullscreenState(fullscreen,
									nullptr);

}

///Return the default DXGI mode for the swap chain
DXGI_SWAP_CHAIN_DESC DX11Output::GetDefaultSwapchainMode() const{

	//Create the swapchain with default settings
	DXGI_SWAP_CHAIN_DESC dxgi_desc;

	ZeroMemory(&dxgi_desc, sizeof(dxgi_desc));

	dxgi_desc.BufferCount = kBuffersCount;
	dxgi_desc.OutputWindow = window_.GetHandle();
	dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgi_desc.Windowed = true;
	dxgi_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//Back buffer and antialiasing - Default
	dxgi_desc.BufferDesc.Format = kVideoFormat;
	dxgi_desc.SampleDesc.Count = 1;

	return dxgi_desc;

}

///Create a new swapchain given its description
void DX11Output::CreateSwapChain(DXGI_SWAP_CHAIN_DESC desc){

	//TODO: Release the outstanding references to the backbuffer in the context

	IDXGISwapChain * swap_chain;

	THROW_ON_FAIL(factory_.CreateSwapChain(&device_,
										   &desc,
										   &swap_chain));

	swap_chain_.reset(swap_chain);

	//TODO: Get the references to the backbuffer

}

///Show the current frame and prepare the next one
void DX11Output::Commit(){

	swap_chain_->Present(IsVSync() ? 1 : 0,
						 0);

}

/////////////////////////////////// MANAGER ///////////////////////////////////////////

unique_ptr<Resource> DX11Manager::LoadDirect(const ResourceKey & key, const void * extras){

	return Loader::GetInstance().Load(key.first, device_, key.second, extras);

}