#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")

#include "dx11/dx11texture.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>
#include <math.h>

#include "exceptions.h"
#include "dx11/dx11shared.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace DirectX;

namespace{

	const float kBitOverByte = 1.0f / 8.0f;
	const float kMIPRatio = 1.0f / 4.0f;
	
}

DX11Texture2D::DX11Texture2D(ID3D11Device & device, const wstring & path){
	
	auto resource = static_cast<ID3D11Resource *>(texture_);

	DDS_ALPHA_MODE alpha_mode;
	
	THROW_ON_FAIL( CreateDDSTextureFromFileEx(&device, 
											  path.c_str(), 
											  0,							// Load everything.
											  D3D11_USAGE_IMMUTABLE, 
											  D3D11_BIND_SHADER_RESOURCE, 
											  0,							// No CPU access.
											  0, 
											  false,						// No forced sRGB
											  &resource, 
											  &view_, 
											  &alpha_mode));				//Alpha informations


	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = BitsPerPixel(description.Format);
	alpha_ = alpha_mode != DDS_ALPHA_MODE_OPAQUE;							//If it is not opaque, it should have an alpha channel

}

DX11Texture2D::~DX11Texture2D(){

	view_->Release();
	texture_->Release();

	view_ = nullptr;
	texture_ = nullptr;

}

size_t DX11Texture2D::GetSize() const{

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical series...

	return static_cast<size_t>( level_size * ((1.0f - std::powf(kMIPRatio, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio)) );

}

ResourcePriority DX11Texture2D::GetPriority() const{

	return EvictionPriorityToResourcePriority(texture_->GetEvictionPriority());

}

void DX11Texture2D::SetPriority(ResourcePriority priority){

	texture_->SetEvictionPriority(ResourcePriorityToEvictionPriority(priority));

}