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

	enum class ResourcePriority;

	namespace dx11{
		
		/// \brief Deleter used by COM IUnknown interface.
		struct COMDeleter{

			/// \brief Release the given resource.
			/// \param ptr Pointer to the resource to delete.
			void operator()(IUnknown * ptr);

		};

		//

		inline void COMDeleter::operator()(IUnknown * ptr){

			ptr->Release();

		}

	}

}