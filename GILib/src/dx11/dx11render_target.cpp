#include "dx11/dx11render_target.h"

#include "graphics.h"
#include "scope_guard.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{

	/// \brief Create a depth stencil texture.
	/// \param width Width of the surface, in pixels.
	/// \param height Height of the surface, in pixels.
	/// \return Returns a pointer to the shader resource view of the surface.
	COMPtr<ID3D11ShaderResourceView> MakeDepthStencil(unsigned int width, unsigned int height){

		ID3D11ShaderResourceView* srv;

		THROW_ON_FAIL(gi_lib::dx11::MakeDepthStencil(*DX11Graphics::GetInstance().GetDevice(),
													 width,
													 height,
													 &srv,
													 nullptr));

		return COMMove(&srv);

	}

	/// \brief Create a render target texture.
	/// \param width Width of the surface, in pixels.
	/// \param height Height of the surface, in pixels.
	/// \param format Format of the surface.
	/// \param mip_chain Whether to create a full MIP chain or not.
	/// \return Returns a pointer to the shader resource view of the surface.
	COMPtr<ID3D11ShaderResourceView> MakeRenderTarget(unsigned int width, unsigned int height, DXGI_FORMAT format, bool mip_chain){

		ID3D11ShaderResourceView* srv;

		THROW_ON_FAIL(gi_lib::dx11::MakeRenderTarget(*DX11Graphics::GetInstance().GetDevice(),
													 width,
													 height,
													 format,
													 &srv,
													 nullptr,
													 mip_chain));
		
		return COMMove(&srv);

	}

}

///////////////////////////// DX11 DEPTH TEXTURE 2D ///////////////////////////////////////

DX11DepthTexture2D::DX11DepthTexture2D(unsigned int width, unsigned int height) :
DX11Texture2D(::MakeDepthStencil(width, 
								 height)){

	ID3D11DepthStencilView* dsv;

	THROW_ON_FAIL(::MakeDepthStencilView(*DX11Graphics::GetInstance().GetDevice(),
										 *GetTexture(),
										 &dsv));

	depth_stencil_view_ << &dsv;

}

void DX11DepthTexture2D::Clear(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil){

	context.ClearDepthStencilView(depth_stencil_view_.Get(), 
								  clear_flags, 
								  depth, 
								  stencil);

}

///////////////////////////// DX11 RENDER TEXTURE 2D ///////////////////////////////////////

DX11RenderTexture2D::DX11RenderTexture2D(unsigned int width, unsigned int height, DXGI_FORMAT format, bool mip_chain) :
DX11Texture2D(::MakeRenderTarget(width, 
								 height, 
								 format, 
								 mip_chain)){

	ID3D11RenderTargetView* rtv;

	THROW_ON_FAIL(::MakeRenderTargetView(*DX11Graphics::GetInstance().GetDevice(),
										 *GetTexture(),
										 &rtv));

	render_target_view_ << &rtv;

}

void DX11RenderTexture2D::Clear(ID3D11DeviceContext& context, Color color){
	
	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	context.ClearRenderTargetView(render_target_view_.Get(),
								  rgba_color);

}

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format){
	
	CreateSurfaces(width,
				   height,
			       target_format);
	
}

DX11RenderTarget::~DX11RenderTarget(){}

bool DX11RenderTarget::Resize(unsigned int width, unsigned int height){

	if (width == GetWidth() && height == GetHeight()){

		return false;

	}

	// Naive approach: discard the old targets and create the new ones preserving the old settings

	std::vector<DXGI_FORMAT> target_format;

	target_format.reserve(render_target_.size());

	for (auto&& texture : render_target_){

		target_format.push_back(texture->GetFormat());

	}

	CreateSurfaces(width,
				   height,
				   target_format);

	return true;

}

void DX11RenderTarget::ClearDepth(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil){

	if (depth_stencil_){

		depth_stencil_->Clear(context,
							  clear_flags,
							  depth,
							  stencil);

	}
	
}

void DX11RenderTarget::ClearTargets(ID3D11DeviceContext& context, Color color){

	for (auto& target : render_target_){

		target->Clear(context,
					  color);
	}

}

void DX11RenderTarget::Bind(ID3D11DeviceContext& context){

	vector<ID3D11RenderTargetView*> rtv_list(render_target_.size());

	std::transform(render_target_.begin(),
				   render_target_.end(),
				   rtv_list.begin(),
				   [](const ObjectPtr<DX11RenderTexture2D>& render_texture){

						return render_texture->GetRenderTargetView().Get();

				   });

	context.OMSetRenderTargets(static_cast<unsigned int>(rtv_list.size()),
							   &rtv_list[0],
							   depth_stencil_->GetDepthStencilView().Get());

	context.RSSetViewports(1,
						   &viewport_);
	
}

void DX11RenderTarget::CreateSurfaces(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format){

	// If the method throws ensures that the resource is left in a clean state.

	auto&& guard = make_scope_guard([this](){

		depth_stencil_ = nullptr;
		render_target_.clear();

	});

	// Create the render target surfaces.

	render_target_.clear();

	for (auto&& format : target_format){

		render_target_.push_back(new DX11RenderTexture2D(width,
														 height,
														 format,
														 false));
	}

	// Depth buffer

	depth_stencil_ = new DX11DepthTexture2D(width,
											height);
	
	// Viewport

	viewport_ = MakeViewport(width,
							 height);
	
	// Cleanup

	guard.Dismiss();

}
