#include "dx11/dx11buffer.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{



}

////////////////////////////// DX11 STRUCTURED VECTOR //////////////////////////////////

DX11DynamicBuffer::DX11DynamicBuffer(const FromDescription& args) :
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

DX11DynamicBuffer::~DX11DynamicBuffer(){

	if (data_){

		delete[] data_;

	}
	
}

void DX11DynamicBuffer::Unlock()
{
	
	dirty_ = true;

}

void DX11DynamicBuffer::Unmap(ID3D11DeviceContext& context){
	
	context.Unmap(buffer_.get(),
				  0);							// Unmap everything.

	dirty_ = false;

}

void DX11DynamicBuffer::Commit(ID3D11DeviceContext& context){

	if (dirty_){

		auto size = static_cast<unsigned int>(element_count_ * element_size_);
		
		memcpy_s(Map<void>(context),
				 size,
				 data_,
				 size);
			
		Unmap(context);


	}

}

void* DX11DynamicBuffer::LockDiscard()
{

	if (dirty_){

		THROW(L"The buffer is already locked. Be sure to unlock the buffer before locking it again.");

	}

	return data_;

}
