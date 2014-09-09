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

	struct NormalTexturedVertex{

		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 binormal;
		XMFLOAT3 target;
		XMFLOAT2 UV;

	};

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

DX11Mesh::DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kFromAttributes> & settings){

	//If exists some attribute which is defined BY_INDEX, the mesh won't used indexing

	if (settings.normal_mapping == AttributeMappingMode::BY_INDEX ||
		settings.binormal_mapping == AttributeMappingMode::BY_INDEX ||
		settings.tangent_mapping == AttributeMappingMode::BY_INDEX ||
		settings.UV_mapping == AttributeMappingMode::BY_INDEX ||
		settings.indices.size() == 0){

		LoadUnindexed(device, settings);

	}
	else
	{

		LoadIndexed(device, settings);

	}

}

void DX11Mesh::LoadIndexed(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kFromAttributes> & settings){

	throw RuntimeException(L"Not implemented yet!");

}

void DX11Mesh::LoadUnindexed(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kFromAttributes> & settings){

#pragma push_macro("max")
#undef max

	//Zip position, normals, binormals, tangents and uvs together.

	vector<NormalTexturedVertex> vertices(std::max(settings.positions.size(),
		settings.normals.size()));

	if (settings.indices.size() > 0){

		// Use the indices to address the components.

	}
	else
	{

		// i-th element of each attribute belongs to the i-th vertex. (everything should be mapped by index).

		auto begin = make_zip(settings.positions.begin(),
							  settings.normals.begin());

		auto end = make_zip(settings.positions.end(),
							settings.normals.end());
		
	}

	// Create the vertex buffer

	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(NormalTexturedVertex) * vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &vertices[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	ID3D11Buffer * vertex_buffer;

	THROW_ON_FAIL(device.CreateBuffer(&bufferDesc, &InitData, &vertex_buffer));

	vertex_buffer_.reset(vertex_buffer);

	
#pragma pop_macro("max")

}

size_t DX11Mesh::GetSize() const{

	return 0;

}

ResourcePriority DX11Mesh::GetPriority() const{

	return ResourcePriority::NORMAL;

}

void DX11Mesh::SetPriority(ResourcePriority priority){


}
