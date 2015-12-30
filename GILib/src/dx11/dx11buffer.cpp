#include "dx11/dx11buffer.h"

#include "dx11/dx11graphics.h"
#include "scope_guard.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{

}

////////////////////////////// DX11 BUFFER /////////////////////////////////

DX11Buffer::DX11Buffer(size_t size, COMPtr<ID3D11Buffer> buffer, COMPtr<ID3D11ShaderResourceView> shader_resource_view) :
size_(size),
buffer_(buffer),
srv_(shader_resource_view){

	data_ = new char[size_];

}

DX11Buffer::~DX11Buffer(){

	delete data_;

}

////////////////////////////// DX11 STRUCTURED BUFFER /////////////////////////////////

DX11StructuredBuffer::DX11StructuredBuffer(const IStructuredBuffer::FromSize& args) :
DX11StructuredBuffer(args.size){}

DX11StructuredBuffer::DX11StructuredBuffer(size_t size){

	ID3D11Buffer* buffer;

	THROW_ON_FAIL(::MakeConstantBuffer(*DX11Graphics::GetInstance().GetDevice(),
									   size,
									   &buffer));

	buffer_ = new DX11Buffer(size,
							 COMMove(&buffer),
							 nullptr);

}

////////////////////////////// DX11 STRUCTURED ARRAY /////////////////////////////////

DX11StructuredArray::DX11StructuredArray(size_t element_count, size_t element_size) :
element_count_(element_count),
element_size_(element_size){

	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* srv;

	THROW_ON_FAIL(::MakeStructuredBuffer(*DX11Graphics::GetInstance().GetDevice(),
										 static_cast<unsigned int>(element_count),
										 static_cast<unsigned int>(element_size),
										 true,
										 &buffer,
										 &srv,
										 nullptr));


	buffer_ = new DX11Buffer(element_count * element_size,
							 COMMove(&buffer),
							 COMMove(&srv));
	
}

///////////////////////////////// DX11 SCRATCH STRUCTURED ARRAY ////////////////////////////////////

DX11ScratchStructuredArray::DX11ScratchStructuredArray(const FromElementSize& arguments):
element_count_(arguments.element_count),
element_size_(arguments.element_size){

	ID3D11Buffer* buffer;
	ID3D11Buffer* staging;

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	auto& device = *DX11Graphics::GetInstance().GetDevice();



	THROW_ON_FAIL(::MakeStagingBuffer(device,
									  static_cast<unsigned int>(element_count_),
									  static_cast<unsigned int>(element_size_),
									  true,
									  &staging));

	auto guard = make_scope_guard([&staging] {

		if (staging) {

			staging->Release();

		}

	});

	THROW_ON_FAIL(::MakeStructuredBuffer(device,
										 static_cast<unsigned int>(element_count_),
										 static_cast<unsigned int>(element_size_),
										 false,
										 &buffer,
										 &srv,
										 &uav));

	shader_resource_view_ = COMMove(&srv);
	unordered_access_view_ = COMMove(&uav);
	buffer_ = COMMove(&buffer);
	readback_buffer_ = COMMove(&staging);

	raw_buffer_ = new char[element_size_ * element_count_];

	guard.Dismiss();

}

void DX11ScratchStructuredArray::Refresh(ID3D11DeviceContext& context) {

	// Read back the result into staging memory

	context.CopyResource(readback_buffer_.Get(), buffer_.Get());		

	// Copy the staging memory buffer into system memory buffer

	D3D11_MAPPED_SUBRESOURCE subresource;

	context.Map(readback_buffer_.Get(),
				0,                                // Map everything
				D3D11_MAP_READ,
				0,
				&subresource);

	auto size = element_size_ * element_count_;

	memcpy_s(raw_buffer_,												
			 size, 
			 subresource.pData, 
			 size);	

	context.Unmap(readback_buffer_.Get(),
				  0);

}