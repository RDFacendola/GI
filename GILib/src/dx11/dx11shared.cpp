#include "dx11/dx11shared.h"

#include <dxgi.h>

#include "graphics.h"
#include "resource.h"
#include "exceptions.h"

namespace gi_lib{

	namespace dx11{

		const DXGI_FORMAT kVideoFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

		AntialiasingMode SampleDescToAntialiasingMode(const DXGI_SAMPLE_DESC & sample_desc){

			if (sample_desc.Count == 1 && sample_desc.Quality == 0)		return AntialiasingMode::NONE;
			if (sample_desc.Count == 2 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_2X;
			if (sample_desc.Count == 4 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_4X;
			if (sample_desc.Count == 8 && sample_desc.Quality == 0)		return AntialiasingMode::MSAA_8X;
			if (sample_desc.Count == 16 && sample_desc.Quality == 0)	return AntialiasingMode::MSAA_16X;

			return AntialiasingMode::NONE;

		}

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

		VideoMode DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi_mode){

			VideoMode video_mode;

			video_mode.horizontal_resolution = dxgi_mode.Width;
			video_mode.vertical_resolution = dxgi_mode.Height;
			video_mode.refresh_rate = static_cast<unsigned int>(std::round(static_cast<float>(dxgi_mode.RefreshRate.Numerator) / dxgi_mode.RefreshRate.Denominator));

			return video_mode;

		}

		unsigned int ResourcePriorityToEvictionPriority(ResourcePriority priority){

			switch (priority){

			case ResourcePriority::MINIMUM:			return DXGI_RESOURCE_PRIORITY_MINIMUM;
			case ResourcePriority::LOW:				return DXGI_RESOURCE_PRIORITY_LOW;
			case ResourcePriority::NORMAL:			return DXGI_RESOURCE_PRIORITY_NORMAL;
			case ResourcePriority::HIGH:			return DXGI_RESOURCE_PRIORITY_HIGH;
			case ResourcePriority::CRITICAL:		return DXGI_RESOURCE_PRIORITY_MAXIMUM;

			}

			throw RuntimeException(L"Unrecognized priority level.");

		}

		ResourcePriority EvictionPriorityToResourcePriority(unsigned int priority){

			switch (priority){

			case DXGI_RESOURCE_PRIORITY_MINIMUM:	return ResourcePriority::MINIMUM;
			case DXGI_RESOURCE_PRIORITY_LOW:		return ResourcePriority::LOW;
			case DXGI_RESOURCE_PRIORITY_NORMAL:		return ResourcePriority::NORMAL;
			case DXGI_RESOURCE_PRIORITY_HIGH:		return ResourcePriority::HIGH;
			case DXGI_RESOURCE_PRIORITY_MAXIMUM:	return ResourcePriority::CRITICAL;

			}

			throw RuntimeException(L"Unrecognized priority level.");

		}

		void COMDeleter::operator()(IUnknown * ptr){

			ptr->Release();

			OutputDebugStringW(L"Releasing control\n");

		}

	}

}