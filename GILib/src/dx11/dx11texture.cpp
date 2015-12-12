#include "dx11/dx11texture.h"

#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")

#include <DDSTextureLoader.h>
#include <DirectXTex.h>

#include "dx11/dx11.h"
#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;
using namespace ::DirectX;

namespace{

	/// \brief Ratio between a Bit and a Byte size.
	const float kBitOverByte = 0.125f;

	/// \brief Size ration between two consecutive MIP levels of a texture 2D.
	const float kMIPRatio2D = 0.25f;
	
}

////////////////////////////// DX11 TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(const FromFile& bundle){
	
	auto device = DX11Graphics::GetInstance().GetDevice();

	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource* texture;
	ID3D11ShaderResourceView* srv;

	THROW_ON_FAIL(CreateDDSTextureFromFileEx(device.Get(),
											 bundle.file_name.c_str(), 
											 0,										// Load everything.
											 D3D11_USAGE_IMMUTABLE, 
											 D3D11_BIND_SHADER_RESOURCE, 
											 0,										// No CPU access.
											 0,
											 false,									// No forced sRGB
											 &texture,
											 &srv,
											 &alpha_mode) );						//Alpha infos

	// Transfer resource's ownership
	shader_resource_view_ << &srv;

	D3D11_TEXTURE2D_DESC description;

	static_cast<ID3D11Texture2D*>(texture)->GetDesc(&description);

	UpdateDescription(description);
	
	texture->Release();		// No longer needed

}

DX11Texture2D::DX11Texture2D(const COMPtr<ID3D11ShaderResourceView>& shader_resource_view) :
shader_resource_view_(shader_resource_view){

	ID3D11Resource* texture;

	shader_resource_view_->GetResource(&texture);

	COM_GUARD(texture);

	D3D11_TEXTURE2D_DESC description;

	static_cast<ID3D11Texture2D*>(texture)->GetDesc(&description);

	UpdateDescription(description);

}

size_t DX11Texture2D::GetSize() const{

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical serie...

	return static_cast<size_t>( level_size * ((1.0f - std::powf(kMIPRatio2D, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio2D)) );

}

void DX11Texture2D::UpdateDescription(const D3D11_TEXTURE2D_DESC& description){
	
	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = static_cast<unsigned int>(BitsPerPixel(description.Format));
	format_ = DXGIFormatToTextureFormat(description.Format);
	
}

////////////////////////////// DX11 GP TEXTURE 2D ////////////////////////////////////////

DX11GPTexture2D::DX11GPTexture2D(const IGPTexture2D::FromDescription& args){

	ID3D11UnorderedAccessView* uav;
	ID3D11ShaderResourceView* srv;

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	THROW_ON_FAIL(::MakeUnorderedTexture(device,		
										 args.width,
										 args.height,
										 TextureFormatToDXGIFormat(args.format),
										 &uav,
										 &srv,
										 args.mips));
	
	texture_ = new DX11Texture2D(COMMove(&srv));

	unordered_access_view_ << &uav;
	
}

///////////////////////////// DX11 TEXTURE 2D ARRAY ////////////////////////////////

DX11Texture2DArray::DX11Texture2DArray(const COMPtr<ID3D11ShaderResourceView>& shader_resource_view) :
	shader_resource_view_(shader_resource_view){

	ID3D11Resource* texture;

	shader_resource_view_->GetResource(&texture);

	COM_GUARD(texture);

	D3D11_TEXTURE2D_DESC description;

	static_cast<ID3D11Texture2D*>(texture)->GetDesc(&description);

	// Update texture info

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = static_cast<unsigned int>(BitsPerPixel(description.Format));
	format_ = DXGIFormatToTextureFormat(description.Format);
	count_ = description.ArraySize;

}

size_t DX11Texture2DArray::GetSize() const {

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical serie...

	auto element_size = static_cast<size_t>(level_size * ((1.0f - std::powf(kMIPRatio2D, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio2D)));

	return element_size * count_;

}

///////////////////////////// DX11 GP TEXTURE 2D ARRAY ////////////////////////////////

DX11GPTexture2DArray::DX11GPTexture2DArray(const ITexture2DArray::FromDescription& args) {

	ID3D11UnorderedAccessView* uav;
	ID3D11ShaderResourceView* srv;

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	THROW_ON_FAIL(::MakeUnorderedTextureArray(device,
											  args.width,
											  args.height,
											  args.count,
											  TextureFormatToDXGIFormat(args.format),
											  &uav,
											  &srv,
											  args.mips));

	texture_array_ = new DX11Texture2DArray(COMMove(&srv));

	unordered_access_view_ << &uav;

}

///////////////////////////// FREE FUNCTIONS /////////////////////////////

DXGI_FORMAT gi_lib::dx11::TextureFormatToDXGIFormat(const TextureFormat& texture_format) {

	switch (texture_format) {

	case TextureFormat::RGBA_HALF:

		return DXGI_FORMAT_R16G16B16A16_FLOAT;

	case TextureFormat::RGBA_FLOAT:

		return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case TextureFormat::RGBA_HALF_UNORM:

		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case TextureFormat::BGRA_HALF_UNORM:

		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case TextureFormat::RGB_FLOAT:

		return DXGI_FORMAT_R11G11B10_FLOAT;

	case TextureFormat::RG_HALF:

		return DXGI_FORMAT_R16G16_FLOAT;

	case TextureFormat::RG_FLOAT:

		return DXGI_FORMAT_R32G32_FLOAT;

	case TextureFormat::DEPTH_STENCIL:

		return DXGI_FORMAT_R24G8_TYPELESS;

	case TextureFormat::BC3_UNORM:

		return DXGI_FORMAT_BC3_UNORM;

	default:

		return DXGI_FORMAT_UNKNOWN;

	}

}

TextureFormat gi_lib::dx11::DXGIFormatToTextureFormat(const DXGI_FORMAT& texture_format) {

	switch (texture_format) {

	case DXGI_FORMAT_R16G16B16A16_FLOAT:

		return TextureFormat::RGBA_HALF;

	case DXGI_FORMAT_R32G32B32A32_FLOAT:

		return TextureFormat::RGBA_FLOAT;

	case DXGI_FORMAT_R8G8B8A8_UNORM:

		return TextureFormat::RGBA_HALF_UNORM;

	case DXGI_FORMAT_B8G8R8A8_UNORM:

		return TextureFormat::BGRA_HALF_UNORM;

	case DXGI_FORMAT_R11G11B10_FLOAT:

		return TextureFormat::RGB_FLOAT;

	case DXGI_FORMAT_R16G16_FLOAT:

		return TextureFormat::RG_HALF;

	case DXGI_FORMAT_R32G32_FLOAT:

		return TextureFormat::RG_FLOAT;

	case DXGI_FORMAT_R24G8_TYPELESS:

		return TextureFormat::DEPTH_STENCIL;

	case DXGI_FORMAT_BC3_UNORM:

		return TextureFormat::BC3_UNORM;

	default:

		THROW(L"Unsupported texture format");

	}

}