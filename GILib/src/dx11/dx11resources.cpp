
#pragma comment(lib,"dxguid.lib")

#include "dx11/dx11resources.h"

#include <set>
#include <map>
#include <unordered_map>
#include <math.h>


#include <DirectXMath.h>
#include <Eigen/Core>

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

////////////////////////////// DX11 STRUCTURED VECTOR //////////////////////////////////

DX11StructuredVector::DX11StructuredVector(const FromDescription& args) :
element_count_(args.element_count),
element_size_(args.element_size),
dirty_(false){

	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* shader_view;

	THROW_ON_FAIL(MakeStructuredBuffer(DX11Graphics::GetInstance().GetDevice(),
									   static_cast<unsigned int>(element_count_),
									   static_cast<unsigned int>(element_size_),
									   true,
									   &buffer,
									   &shader_view,
									   nullptr));

	data_ = new char[element_size_ * element_count_];

	buffer_.reset(buffer);
	shader_view_.reset(shader_view);
	
}

DX11StructuredVector::~DX11StructuredVector(){

	if (data_){

		delete[] data_;

	}
	
}

void DX11StructuredVector::Unlock()
{
	
	dirty_ = true;

}

void DX11StructuredVector::Unmap(ID3D11DeviceContext& context){
	
	context.Unmap(buffer_.get(),
				  0);							// Unmap everything.

	dirty_ = false;

}

void DX11StructuredVector::Commit(ID3D11DeviceContext& context){

	if (dirty_){

		auto size = static_cast<unsigned int>(element_count_ * element_size_);
		
		memcpy_s(Map<void>(context),
				 size,
				 data_,
				 size);
			
		Unmap(context);


	}

}

void* DX11StructuredVector::LockDiscard()
{

	if (dirty_){

		THROW(L"The buffer is already locked. Be sure to unlock the buffer before locking it again.");

	}

	return data_;

}
