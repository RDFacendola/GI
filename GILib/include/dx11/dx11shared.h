/// \file dx11shared.h
/// \brief Shared and utility classes and methods for DirectX11
///
/// \author Raffaele D. Facendola

#include <dxgi.h>

#include "resources.h"
#include "exceptions.h"

namespace gi_lib{

	namespace dx11{

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
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

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
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

	}

}