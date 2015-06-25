#include "dx11/dx11render_target.h"

#include "dx11/dx11.h"
#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(ID3D11Texture2D & target){

	zstencil_ = nullptr;
	zstencil_view_ = nullptr;
	
	SetBuffers({ &target });

}

DX11RenderTarget::DX11RenderTarget(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format, bool unordered_access){
	
	zstencil_ = nullptr;
	zstencil_view_ = nullptr;

	unordered_access_ = unordered_access;

	Initialize(width,
			   height,
			   target_format);
	
}

DX11RenderTarget::~DX11RenderTarget(){

	ResetBuffers();

}

void DX11RenderTarget::SetBuffers(std::initializer_list<ID3D11Texture2D*> targets){

	/// The render target view format and the shader resource view format for the render targets are the same of the textures they are generated from (DXGI_FORMAT_UNKNOWN).
	/// The depth stencil texture is created with a 24bit channel for the depth and a 8bit channel for the stencil, both without a type (DXGI_FORMAT_R24G8_TYPELESS).
	/// The depth stencil view format of the depth stencil texture is 24bit uniform for the depth and 8bit unsigned int for the stencil (DXGI_FORMAT_D24_UNORM_S8_UINT).
	/// The shader resource view of the depth stencil texture is 24bit uniform for the depth. The stencil cannot be sampled inside the shader (DXGI_FORMAT_R24_UNORM_X8_TYPELESS).
	
	ResetBuffers();

	ID3D11Device * device;
	
	ID3D11Texture2D * zstencil;
	D3D11_TEXTURE2D_DESC desc;

	ID3D11RenderTargetView * render_target_view;

	auto& target = **targets.begin();

	// Rollback guard ensures that the state of the render target is cleared on error
	// (i.e.: if one buffer causes an exception, the entire operation is rollback'd)

	auto rollback = make_scope_guard([this](){
	
		textures_.clear();
		target_views_.clear();

		zstencil_ = nullptr;
		zstencil_view_ = nullptr;
	
	});
	
	target.GetDevice(&device);

	COM_GUARD(device);

	for (auto target : targets){
		
		THROW_ON_FAIL(device->CreateRenderTargetView(reinterpret_cast<ID3D11Resource *>(target),
													 nullptr,
													 &render_target_view));

		textures_.push_back(new DX11Texture2D(*target, 
											  DXGI_FORMAT_UNKNOWN));

		target_views_.push_back(render_target_view);

	}

	// Create the z-stencil and the z-stencil view
		
	target.GetDesc(&desc);

	THROW_ON_FAIL(MakeDepthStencil(*device, 
								   desc.Width, 
								   desc.Height, 
								   &zstencil, 
								   &zstencil_view_));

	zstencil_ = new DX11Texture2D(*zstencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);					// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource
	
	// Update the viewport

	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.Width = static_cast<float>(desc.Width);
	viewport_.Height = static_cast<float>(desc.Height);

	// Everything went as it should have...
	rollback.Dismiss();

}

bool DX11RenderTarget::Resize(unsigned int width, unsigned int height){

	if (width == GetWidth() && height == GetHeight()){

		return false;

	}

	// Naive approach: discard the old targets and create the new ones preserving the old settings

	std::vector<DXGI_FORMAT> target_format;

	for (auto&& texture : textures_){

		target_format.push_back(texture->GetFormat());

	}

	Initialize(width, 
			   height,
			   target_format);

	return true;

}

void DX11RenderTarget::ResetBuffers(){

	// Release the targets

	textures_.clear();
	
	// Release the target views

	for (auto&& target_view : target_views_){

		target_view->Release();

	}

	target_views_.clear();

	// Release the zstencil

	zstencil_ = nullptr;

	// Release the zstencil view

	if (zstencil_view_){

		zstencil_view_->Release();

		zstencil_view_ = nullptr;

	}
	
}

void DX11RenderTarget::ClearDepthStencil(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil){

	context.ClearDepthStencilView(zstencil_view_, clear_flags, depth, stencil);
	
}

void DX11RenderTarget::ClearTargets(ID3D11DeviceContext& context, Color color){

	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	for (auto & target_view : target_views_){

		context.ClearRenderTargetView(target_view, 
									  rgba_color);

	}

}

void DX11RenderTarget::Bind(ID3D11DeviceContext& context){

	context.OMSetRenderTargets(static_cast<unsigned int>(target_views_.size()),
							   &target_views_[0],
							   zstencil_view_);

	context.RSSetViewports(1,
						   &viewport_);
	
}

void DX11RenderTarget::Initialize(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format){

	ResetBuffers();

	// If the method throws ensures that the resource is left in a clear state.
	auto&& guard = make_scope_guard([this](){

		ResetBuffers();

	});

	auto&& device = DX11Graphics::GetInstance().GetDevice();
	   
	ID3D11Texture2D* texture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav = nullptr;
		
	// Create the render target surfaces.

	for (auto&& format : target_format){

		THROW_ON_FAIL(MakeRenderTarget(device,
									   width,
									   height,
 									   format,
									   &texture,
									   &rtv,
									   &srv,
									   unordered_access_ ? &uav : nullptr));
		
		if (uav){

			textures_.push_back(new DX11Texture2D(*texture,
												  *srv,
												  *uav));

		}
		else{

			textures_.push_back(new DX11Texture2D(*texture,
												  *srv));

		}
		
		target_views_.push_back(rtv);

	}

	// Depth stencil

	ID3D11Texture2D* zstencil;

	THROW_ON_FAIL(MakeDepthStencil(device,
								   width,
								   height,
								   &zstencil,
								   &zstencil_view_));
		
	zstencil_ = new DX11Texture2D(*zstencil, 
								  DXGI_FORMAT_R24_UNORM_X8_TYPELESS);	// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource

	// Update the viewport

	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	
	guard.Dismiss();

}
