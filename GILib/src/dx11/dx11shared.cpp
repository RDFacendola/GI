#include "dx11/dx11shared.h"

#include <dxgi.h>

#include "graphics.h"
#include "resource.h"
#include "exceptions.h"

namespace gi_lib{

	namespace dx11{



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