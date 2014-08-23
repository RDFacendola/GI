#include "dx11/dx11shared.h"

#include "exceptions.h"

using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

/// \brief Convert a resource priority to an eviction priority (DirectX11)
unsigned int gi_lib::dx11::ResourcePriorityToEvictionPriority(ResourcePriority priority){

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
ResourcePriority gi_lib::dx11::EvictionPriorityToResourcePriority(unsigned int priority){

	switch (priority){

	case DXGI_RESOURCE_PRIORITY_MINIMUM:	return ResourcePriority::MINIMUM;
	case DXGI_RESOURCE_PRIORITY_LOW:		return ResourcePriority::LOW;
	case DXGI_RESOURCE_PRIORITY_NORMAL:		return ResourcePriority::NORMAL;
	case DXGI_RESOURCE_PRIORITY_HIGH:		return ResourcePriority::HIGH;
	case DXGI_RESOURCE_PRIORITY_MAXIMUM:	return ResourcePriority::CRITICAL;

	}

	throw RuntimeException(L"Unrecognized priority level.");

}