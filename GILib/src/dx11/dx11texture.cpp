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
	
	/// \brief Convert a texture format to a DXGI format used by DirectX11.
	/// \param texture_format Texture format to convert.
	/// \return Returns the DXGI format corresponding to the texture format specified. If no conversion could be performed, returns DXGI_FORMAT_UNKNOWN.
	DXGI_FORMAT TextureFormatToDXGIFormat(const TextureFormat& texture_format){

		switch (texture_format){

		case TextureFormat::RGBA_HALF:

			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		default:
			
			return DXGI_FORMAT_UNKNOWN;

		}

	}

}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

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
	format_ = description.Format;
	
}

////////////////////////////// GP TEXTURE 2D ////////////////////////////////////////

DX11GPTexture2D::DX11GPTexture2D(unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int mips){

	ID3D11UnorderedAccessView* uav;
	ID3D11ShaderResourceView* srv;

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	THROW_ON_FAIL(::MakeUnorderedTexture(device,		
										 width,
										 height,
										 format,
										 &uav,
										 &srv,
										 mips));

	texture_ = new DX11Texture2D(COMMove(&srv));

	unordered_access_view_ << &uav;
	
}

