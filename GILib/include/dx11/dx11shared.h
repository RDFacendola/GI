/// \file dx11shared.h
/// \brief Shared and utility classes and methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

struct IUnknown;

struct DXGI_SAMPLE_DESC;
struct DXGI_MODE_DESC;
enum DXGI_FORMAT;

namespace gi_lib{

	enum class AntialiasingMode;
	enum class ResourcePriority;

	struct VideoMode;
	
	namespace dx11{
		
		/// \brief Format of the back buffer.
		extern const DXGI_FORMAT kVideoFormat;

		/// \brief Deleter used by COM IUnknown interface.
		struct COMDeleter{

			/// \brief Release the given resource.
			/// \param ptr Pointer to the resource to delete.
			void operator()(IUnknown * ptr);

		};

		/// Convert a multisample structure to an antialiasing mode
		AntialiasingMode SampleDescToAntialiasingMode(const DXGI_SAMPLE_DESC & sample_desc);

		/// Convert an antialiasing mode to a multisample structure
		DXGI_SAMPLE_DESC AntialiasingModeToSampleDesc(const AntialiasingMode & antialiasing_mode);

		/// Convert a video mode to a mode desc
		DXGI_MODE_DESC VideoModeToDXGIMode(const VideoMode & video_mode);

		/// Convert a mode desc to a video mode
		VideoMode DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi_mode);

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		unsigned int ResourcePriorityToEvictionPriority(ResourcePriority priority);

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		ResourcePriority EvictionPriorityToResourcePriority(unsigned int priority);

	}

}