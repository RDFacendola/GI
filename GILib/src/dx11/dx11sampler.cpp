#include "dx11/dx11sampler.h"

#include "dx11/dx11.h"
#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{

	D3D11_TEXTURE_ADDRESS_MODE TextureMappingToAddressMode(TextureMapping mapping) {

		return mapping == TextureMapping::CLAMP ?
						  D3D11_TEXTURE_ADDRESS_CLAMP :
						  D3D11_TEXTURE_ADDRESS_WRAP;

	}

	D3D11_FILTER TextureFilteringToFilter(TextureFiltering filtering) {

		switch (filtering) {

		case TextureFiltering::NEAREST:

			return D3D11_FILTER_MIN_MAG_MIP_POINT;
			
		case TextureFiltering::BILINEAR:

			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		case TextureFiltering::TRILINEAR:

			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			
		case TextureFiltering::ANISOTROPIC:
			
			return D3D11_FILTER_ANISOTROPIC;

		case TextureFiltering::PERCENTAGE_CLOSER:
			
			return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

		default:
			
			THROW(L"Unsuppored filtering!");

		}

	}

}

//////////////////////////////// DIRECTX11 SAMPLER ////////////////////////////////

DX11Sampler::DX11Sampler(const FromDescription& description){

	ID3D11SamplerState* sampler_state;

	auto& device = *DX11Graphics::GetInstance().GetDevice();

	if (description.texture_filtering == TextureFiltering::PERCENTAGE_CLOSER) {

		// Percentage-closer filtering

		THROW_ON_FAIL(MakePCFSampler(device,
									 TextureMappingToAddressMode(description.texture_mapping),
									 &sampler_state));

	}
	else {

		// Normal sampler
		
		THROW_ON_FAIL(MakeSampler(device,
								  TextureMappingToAddressMode(description.texture_mapping),
								  TextureFilteringToFilter(description.texture_filtering),
								  description.anisotropy_level,
								  Vector4f::Zero(),
								  &sampler_state));

	}

	texture_filtering_ = description.texture_filtering;
	texture_mapping_ = description.texture_mapping;
	max_anisotropy_ = description.anisotropy_level;
	
	sampler_state_ << &sampler_state;

}