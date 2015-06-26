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

	THROW_ON_FAIL(MakeSampler(DX11Graphics::GetInstance().GetDevice(),
							  description.texture_mapping,
							  description.anisotropy_level,
							  &sampler_state));

	sampler_state_.reset(sampler_state);

}

