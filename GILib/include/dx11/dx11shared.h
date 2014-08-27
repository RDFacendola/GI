/// \file dx11shared.h
/// \brief Shared and utility classes and methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

struct IUnknown;

namespace gi_lib{

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