#include "dx11/dx11sampler.h"

#include "dx11/dx11.h"
#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{



}

//////////////////////////////// DIRECTX11 SAMPLER ////////////////////////////////

DX11Sampler::DX11Sampler(const FromDescription& description){

	ID3D11SamplerState* sampler_state;

	auto adddress_mode = description.texture_mapping == TextureMapping::WRAP ? 
						 D3D11_TEXTURE_ADDRESS_WRAP : 
						 D3D11_TEXTURE_ADDRESS_CLAMP;

	THROW_ON_FAIL(MakeSampler(*DX11Graphics::GetInstance().GetDevice(),
							  adddress_mode,
							  description.anisotropy_level,
							  Vector4f::Zero(),
							  &sampler_state));

	sampler_state_ << &sampler_state;

	texture_mapping_ = description.texture_mapping;
	max_anisotropy_ = description.anisotropy_level;

}