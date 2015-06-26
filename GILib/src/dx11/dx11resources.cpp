
#pragma comment(lib,"dxguid.lib")

#include "dx11/dx11resources.h"

#include <set>
#include <map>
#include <unordered_map>
#include <math.h>


#include <DirectXMath.h>

#include "gimath.h"
#include "core.h"
#include "enums.h"
#include "exceptions.h"
#include "scope_guard.h"
#include "observable.h"

#include "dx11/dx11.h"
#include "dx11/dx11graphics.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

using namespace DirectX;
using namespace Eigen;

namespace{


}

//////////////////////////////// DIRECTX11 SAMPLER ////////////////////////////////

size_t DX11Sampler::FromDescription::GetCacheKey() const{

	// | ... | texture_mapping | anisotropy_level |
	//      40                 8                  0

	return (anisotropy_level & 0xFF) | (static_cast<unsigned int>(texture_mapping) << 8);

}

DX11Sampler::DX11Sampler(const FromDescription& description){

	ID3D11SamplerState* sampler_state;

	THROW_ON_FAIL(MakeSampler(DX11Graphics::GetInstance().GetDevice(),
							  description.texture_mapping,
							  description.anisotropy_level,
							  &sampler_state));

	sampler_state_.reset(sampler_state);

}

