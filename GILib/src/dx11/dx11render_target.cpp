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

DX11RenderTexture2D::DX11RenderTexture2D(const COMPtr<ID3D11RenderTargetView>& render_target_view, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view) :
	DX11Texture2D(shader_resource_view),
	render_target_view_(render_target_view){

	ID3D11Resource* render_target;

	render_target_view_->GetResource(&render_target);

	COM_GUARD(render_target);

	D3D11_TEXTURE2D_DESC description;

	static_cast<ID3D11Texture2D*>(render_target)->GetDesc(&description);

	mip_chain_ = description.MipLevels > 1;
	
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

DX11RenderTarget::DX11RenderTarget(const IRenderTarget::FromDescription& args){
	
	CreateSurfaces(args.width,
				   args.height,
			       args.format,
				   args.depth);
	
}

DX11RenderTarget::DX11RenderTarget(const COMPtr<ID3D11RenderTargetView>& render_target_view) :
depth_stencil_(nullptr){

	// Create the SRV from the RTV

	auto device = DX11Graphics::GetInstance().GetDevice();

	ID3D11Resource* resource;
	ID3D11ShaderResourceView* srv;

	render_target_view->GetResource(&resource);

	COM_GUARD(resource);

	THROW_ON_FAIL(device->CreateShaderResourceView(resource,
												   nullptr,
												   &srv));

	render_target_.push_back(new DX11RenderTexture2D(render_target_view,
													 COMMove(&srv)));

	// Viewport

	viewport_ = MakeViewport(render_target_[0]->GetWidth(),
							 render_target_[0]->GetHeight());
	
}

DX11RenderTarget::~DX11RenderTarget(){}

vector<TextureFormat> DX11RenderTarget::GetFormat() const {

	vector<TextureFormat> formats;

	formats.reserve(render_target_.size());

	for (auto&& surface : render_target_) {

		formats.push_back(surface->GetFormat());

	}

	return formats;

}

bool DX11RenderTarget::Resize(unsigned int width, unsigned int height){

	if (width == GetWidth() && height == GetHeight()){

		return false;

	}

	// Naive approach: discard the old targets and create the new ones preserving the old settings

	std::vector<TextureFormat> target_format;

	target_format.reserve(render_target_.size());

	for (auto&& texture : render_target_){

		target_format.push_back(texture->GetFormat());

	}

	CreateSurfaces(width,
				   height,
				   target_format,
				   depth_stencil_ != nullptr);

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

	vector<ID3D11UnorderedAccessView*> uav_list;
	
	Bind(context, uav_list);

}

void DX11RenderTarget::Bind(ID3D11DeviceContext& context, const vector<ID3D11UnorderedAccessView*>& uav_list) {

	vector<ID3D11RenderTargetView*> rtv_list(render_target_.size());

	std::transform(render_target_.begin(),
				   render_target_.end(),
				   rtv_list.begin(),
				   [](const ObjectPtr<DX11RenderTexture2D>& render_texture){

						return render_texture->GetRenderTargetView().Get();

				   });

	ID3D11RenderTargetView** rtv = (rtv_list.size() == 0) ? nullptr : &rtv_list[0];
	ID3D11DepthStencilView* dsv = (depth_stencil_) ? depth_stencil_->GetDepthStencilView().Get() : nullptr;

	if (uav_list.size() > 0) {

		// Bind both the render target and the UAVs

		context.OMSetRenderTargetsAndUnorderedAccessViews(static_cast<unsigned int>(rtv_list.size()),
														  rtv,
														  dsv,
														  static_cast<unsigned int>(rtv_list.size()),
														  static_cast<unsigned int>(uav_list.size()),
														  &uav_list[0],
														  nullptr);
		
	}
	else {

		// Bind the render targets only

		context.OMSetRenderTargets(static_cast<unsigned int>(rtv_list.size()),
								   rtv,
								   dsv);

	}

	context.RSSetViewports(1, &viewport_);

}

void DX11RenderTarget::Unbind(ID3D11DeviceContext& context){

	vector<ID3D11RenderTargetView*> rtv_null_list(render_target_.size(), nullptr);

	context.OMSetRenderTargets(static_cast<unsigned int>(rtv_null_list.size()),
							   &rtv_null_list[0],
							   nullptr);

}

void DX11RenderTarget::Unbind(ID3D11DeviceContext& context, const vector<ID3D11UnorderedAccessView*>& uav_list){

	vector<ID3D11RenderTargetView*> rtv_null_list(render_target_.size(), nullptr);
	vector<ID3D11UnorderedAccessView*> null_uav(uav_list.size(), nullptr);

	context.OMSetRenderTargetsAndUnorderedAccessViews(static_cast<unsigned int>(rtv_null_list.size()),
													  rtv_null_list.size() > 0 ? &rtv_null_list[0] : nullptr,
													  nullptr,
													  static_cast<unsigned int>(render_target_.size()),
													  static_cast<unsigned int>(null_uav.size()),
													  null_uav.size() > 0 ? &null_uav[0] : nullptr,
													  nullptr);
	
}

void DX11RenderTarget::CreateSurfaces(unsigned int width, unsigned int height, const vector<TextureFormat>& target_format, bool depth){

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
														 TextureFormatToDXGIFormat(format),
														 false));
	}

	// Depth buffer

	if (depth) {

		depth_stencil_ = new DX11DepthTexture2D(width,
												height);

	}
	else {

		depth_stencil_ = nullptr;

	}
	
	// Viewport

	viewport_ = MakeViewport(width,
							 height);
	
	// Cleanup

	guard.Dismiss();

}

///////////////////////////// RENDER TARGET CACHE ///////////////////////////////////////

std::vector<ObjectPtr<DX11RenderTarget>> DX11RenderTargetCache::cache_;

DX11RenderTargetCache::DX11RenderTargetCache(const Singleton&) {}

void DX11RenderTargetCache::PushToCache(const ObjectPtr<IRenderTarget>& texture) {

	if (texture != nullptr) {
	
		cache_.push_back(resource_cast(texture));

	}

}

ObjectPtr<IRenderTarget> DX11RenderTargetCache::PopFromCache(unsigned int width, unsigned int height, vector<TextureFormat> format, bool has_depth, bool generate) {

	auto it = std::find_if(cache_.begin(),
						   cache_.end(),
						   [width, height, format, has_depth](const ObjectPtr<DX11RenderTarget>& cached_texture) {

								return width == cached_texture->GetWidth() &&
									   height == cached_texture->GetHeight() &&
									   (has_depth ^ (cached_texture->GetDepthBuffer() == nullptr)) &&
									   format == cached_texture->GetFormat();

						   });

	if (it != cache_.end()) {

		auto ptr = *it;

		cache_.erase(it);

		return ObjectPtr<IRenderTarget>(ptr);
		
	}
	else if (generate) {

		return new DX11RenderTarget(IRenderTarget::FromDescription{ width, height, format, has_depth });

	}
	else {

		return nullptr;

	}

}

void DX11RenderTargetCache::PurgeCache() {

	cache_.clear();

}

size_t DX11RenderTargetCache::GetSize() const {

	size_t size = 0;

	for (auto&& cached_texture : cache_) {

		size += cached_texture->GetSize();

	}

	return size;

}

////////////////////////////// DX11 RENDER TARGET ARRAY ////////////////////////////////////

DX11RenderTargetArray::~DX11RenderTargetArray() {}

DX11RenderTargetArray::DX11RenderTargetArray(const IRenderTargetArray::FromDescription& args) {

	// If the method throws ensures that the resource is left in a clean state.

	auto&& guard = make_scope_guard([this]() {

		depth_stencil_ = nullptr;
		render_target_array_ = nullptr;
		rtv_list_.clear();

	});
	
	// Create the render target surfaces.
	ID3D11ShaderResourceView* srv;
	vector<ID3D11RenderTargetView*> rtv_list;

	THROW_ON_FAIL(MakeRenderTargetArray(*DX11Graphics::GetInstance().GetDevice(),
										args.width,
										args.height,
										args.count,
										TextureFormatToDXGIFormat(args.format),
										&srv,
										&rtv_list,
										false));
										
	render_target_array_ = new DX11Texture2DArray(COMMove(&srv));
							
	rtv_list_.reserve(rtv_list.size());

	for (auto&& render_target_view : rtv_list) {

		rtv_list_.push_back(COMMove(&render_target_view));

	}

	// Depth buffer

	depth_stencil_ = new DX11DepthTexture2D(args.width,
											args.height);

	// Viewport

	viewport_ = MakeViewport(args.width,
							 args.height);

	// Cleanup

	guard.Dismiss();

}

void DX11RenderTargetArray::ClearDepth(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil) {

	if (depth_stencil_) {

		depth_stencil_->Clear(context,
							  clear_flags,
							  depth,
							  stencil);

	}

}

void DX11RenderTargetArray::ClearTargets(ID3D11DeviceContext& context, Color color) {

	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	for (auto& render_target_view : rtv_list_) {

		context.ClearRenderTargetView(render_target_view.Get(),
									  rgba_color);

	}

}

void DX11RenderTargetArray::Bind(ID3D11DeviceContext& context, unsigned int index, D3D11_VIEWPORT* viewport) {

	vector<ID3D11RenderTargetView*> rtv_list(1, rtv_list_[index].Get());

	if (!viewport) {

		viewport = &viewport_;

	}

	if (depth_stencil_) {

		// Depth stencil

		context.OMSetRenderTargets(static_cast<unsigned int>(rtv_list.size()),
								   &rtv_list[0],
								   depth_stencil_->GetDepthStencilView().Get());

	}
	else {

		// No depth stencil here

		context.OMSetRenderTargets(static_cast<unsigned int>(rtv_list.size()),
								   &rtv_list[0],
								   nullptr);

	}

	context.RSSetViewports(1,
						   viewport);

}

void DX11RenderTargetArray::Unbind(ID3D11DeviceContext& context) {

	// Only one surface is actually bound to the context

	vector<ID3D11RenderTargetView*> rtv_null_list(1, nullptr);

	context.OMSetRenderTargets(static_cast<unsigned int>(rtv_null_list.size()),
							   &rtv_null_list[0],
							   nullptr);

	// Remove the optional shaders (the mandatory ones are overwritten anyway)

	context.GSSetShader(nullptr, nullptr, 0);
	context.HSSetShader(nullptr, nullptr, 0);
	context.DSSetShader(nullptr, nullptr, 0);

}