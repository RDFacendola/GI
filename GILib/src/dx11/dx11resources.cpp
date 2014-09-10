#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")

#include "dx11resources.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>
#include <DirectXMath.h>
#include <Eigen/Core>

#include <math.h>

#include "..\..\include\core.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\functional.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace DirectX;

namespace{

	/// \brief Ratio between a Bit and a Byte size.
	const float kBitOverByte = 1.0f / 8.0f;

	/// \brief Size ration between two consecutive MIP levels of a texture 2D.
	const float kMIPRatio2D = 1.0f / 4.0f;
	
	/// \brief Convert a resource priority to an eviction priority
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

	/// \brief Convert a resource priority to an eviction priority (DirectX11)
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
	
	/// \brief Convert an Eigen Vector3f to an XMFLOAT3
	XMFLOAT3 EigenVector3fToXMFLOAT3(const Eigen::Vector3f & vector){

		return XMFLOAT3(vector.x(), 
						vector.y(), 
						vector.z());

	}

	/// \brief Convert an Eigen Vector2f to an XMFLOAT2
	XMFLOAT2 EigenVector2fToXMFLOAT2(const Eigen::Vector2f & vector){

		return XMFLOAT2(vector.x(),
						vector.y());

	}

	/// \brief Create an index buffer
	template <typename TVertexFormat>
	ID3D11Buffer * MakeVertexBuffer(ID3D11Device & device, const vector<TVertexFormat> & vertices){

		ID3D11Buffer * vertex_buffer = nullptr;

		// Fill in a buffer description.
		D3D11_BUFFER_DESC buffer_desc;

		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(TVertexFormat) * vertices.size());
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA init_data;

		init_data.pSysMem = &vertices[0];
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;

		THROW_ON_FAIL(device.CreateBuffer(&buffer_desc, &init_data, &vertex_buffer));

		return vertex_buffer;

	}

	/// \brief Create an index buffer
	ID3D11Buffer * MakeIndexBuffer(ID3D11Device & device, const vector<unsigned int> & indices){

		ID3D11Buffer * index_buffer = nullptr;

		// Fill in a buffer description.
		D3D11_BUFFER_DESC buffer_desc;

		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = static_cast<UINT>( sizeof(unsigned int) * indices.size() );
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = &indices[0];
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;

		// Create the buffer with the device.
		THROW_ON_FAIL(device.CreateBuffer(&buffer_desc, &init_data, &index_buffer));

		return index_buffer;

	}
	
}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(ID3D11Device & device, const LoadSettings<Texture2D, Texture2D::LoadMode::kFromDDS> & settings){
	
	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource * resource;
	ID3D11ShaderResourceView * shader_view;

	wstringstream path;

	path << Application::GetInstance().GetDirectory() << settings.file_name;

	THROW_ON_FAIL( CreateDDSTextureFromFileEx(&device, 
											  path.str().c_str(), 
											  0,									// Load everything.
											  D3D11_USAGE_IMMUTABLE, 
											  D3D11_BIND_SHADER_RESOURCE, 
											  0,									// No CPU access.
											  0,
											  false,								// No forced sRGB
											  &resource,
											  &shader_view, 
											  &alpha_mode) );						//Alpha informations

	texture_.reset(static_cast<ID3D11Texture2D*>(resource));	
	shader_view_.reset(shader_view);

	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = BitsPerPixel(description.Format);
	alpha_ = alpha_mode != DDS_ALPHA_MODE_OPAQUE;									//If it is not opaque, it should have an alpha channel

}

size_t DX11Texture2D::GetSize() const{

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical serie...

	return static_cast<size_t>( level_size * ((1.0f - std::powf(kMIPRatio2D, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio2D)) );

}

ResourcePriority DX11Texture2D::GetPriority() const{

	return EvictionPriorityToResourcePriority(texture_->GetEvictionPriority());

}

void DX11Texture2D::SetPriority(ResourcePriority priority){

	texture_->SetEvictionPriority(ResourcePriorityToEvictionPriority(priority));

}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kPosition> & settings){

	// Position-only mesh

	vertex_buffer_.reset(MakeVertexBuffer(device, settings.vertices));

	if (settings.indices.size() > 0){

		index_buffer_.reset(MakeIndexBuffer(device, settings.indices));
		
		polygon_count_ = settings.indices.size();

	}
	else{

		polygon_count_ = settings.vertices.size() / 3;

	}

	vertex_count_ = settings.vertices.size();
	LOD_count_ = 1;

}

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kTextured> & settings){

	// Textured mesh.

	vertex_buffer_.reset(MakeVertexBuffer(device, settings.vertices));

	if (settings.indices.size() > 0){

		index_buffer_.reset(MakeIndexBuffer(device, settings.indices));

		polygon_count_ = settings.indices.size();

	}
	else{

		polygon_count_ = settings.vertices.size() / 3;

	}

	vertex_count_ = settings.vertices.size();
	LOD_count_ = 1;

}

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kNormalTextured> & settings){

	// Normal, textured mesh.

	vertex_buffer_.reset(MakeVertexBuffer(device, settings.vertices));

	if (settings.indices.size() > 0){

		index_buffer_.reset(MakeIndexBuffer(device, settings.indices));
	
		polygon_count_ = settings.indices.size();

	}
	else{

		polygon_count_ = settings.vertices.size() / 3;

	}

	vertex_count_ = settings.vertices.size();
	LOD_count_ = 1;

}

size_t DX11Mesh::GetSize() const{

	return 0;

}

ResourcePriority DX11Mesh::GetPriority() const{

	return ResourcePriority::NORMAL;

}

void DX11Mesh::SetPriority(ResourcePriority){


}
