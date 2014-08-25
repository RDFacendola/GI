/// \file dx11shared.h
/// \brief Shared and utility classes and methods for DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <dxgi.h>
#include <memory>
#include <type_traits>

#include "resource.h"
#include "exceptions.h"

using ::std::unique_ptr;
using ::std::shared_ptr;

namespace gi_lib{

	namespace dx11{

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		unsigned int ResourcePriorityToEvictionPriority(ResourcePriority priority);

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		ResourcePriority EvictionPriorityToResourcePriority(unsigned int priority);

		/// \brief Deleter used by COM IUnknown interface.
		struct COMDeleter{

			/// \brief Release the given resource.
			/// \param ptr Pointer to the resource to delete.
			inline void operator()(IUnknown * ptr){

				ptr->Release();

			}

		};
		
	}

}