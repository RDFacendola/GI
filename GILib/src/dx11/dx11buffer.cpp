#include "dx11/dx11buffer.h"

#include "dx11/dx11graphics.h"

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