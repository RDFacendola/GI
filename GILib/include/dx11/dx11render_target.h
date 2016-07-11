/// \file dx11render_target.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include <numeric>
#include <vector>

#include "graphics.h"
#include "render_target.h"

#include "dx11/dx11.h"
#include "dx11/dx11texture.h"

#include "windows/win_os.h"

namespace gi_lib{

	union Color;

	namespace dx11{

		using windows::COMPtr;

		/// \brief DirectX11  2D texture that can be used as a depth-stencil buffer.
		/// \author Raffaele D. Facendola.
		class DX11DepthTexture2D : public DX11Texture2D{

		public:
			
			/// \brief Create a new depth texture from explicit dimensions.
			/// \param width The width of the texture.
			/// \param height The height of the texture.
			DX11DepthTexture2D(unsigned int width, unsigned int height);

			/// \brief Virtual destructor.
			virtual ~DX11DepthTexture2D(){}

			/// \brief Clear the depth buffer.
			/// \param context The context used to clear the surface.
			/// \param clear_flags Determines whether to clear the depth and\or the stencil buffer. (see: D3D11_CLEAR_FLAGS)
			/// \param depth Depth value to store inside the depth buffer.
			/// \param stencil Stencil value to store inside the stencil buffer.
			void Clear(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil);

			/// \brief Get the depth stencil view used to bind this texture to the pipeline.
			COMPtr<ID3D11DepthStencilView> GetDepthStencilView() const;

		private:

			COMPtr<ID3D11DepthStencilView> depth_stencil_view_;			///< \brief Pointer to the depth stencil view of the texture.

		};

		/// \brief DirectX11  2D texture that can be used as a render target.
		/// \author Raffaele D. Facendola.
		class DX11RenderTexture2D : public DX11Texture2D{

		public:

			/// \brief Create a new render texture from explicit dimensions.
			/// \param width The width of the texture.
			/// \param height The height of the texture.
			/// \param format Format of the surface.
			/// \param mip_chain Whether the render texture will support full MIP-map chain or not.
			DX11RenderTexture2D(unsigned int width, unsigned int height, DXGI_FORMAT format, bool mip_chain = false);

			/// \brief Create a render texture from a render target view and a shader resource view.
			DX11RenderTexture2D(const COMPtr<ID3D11RenderTargetView>& render_target_view, const COMPtr<ID3D11ShaderResourceView>& shader_resource_view);

			/// \brief Virtual destructor.
			virtual ~DX11RenderTexture2D(){}

			/// \brief Clear the surface.
			/// \param context The context used to clear the surface.
			/// \param color The color used to clear the surface.
			void Clear(ID3D11DeviceContext& context, Color color);

			/// \brief Get the render target view used to bind this texture to the pipeline.
			COMPtr<ID3D11RenderTargetView> GetRenderTargetView() const;

		private:

			COMPtr<ID3D11RenderTargetView> render_target_view_;			///< \brief Pointer to the render target view of the texture.

			bool mip_chain_;											/// Whether the texture will support full MIP-map chain or not.

		};

		/// \brief DirectX11 render target.
		/// \author Raffaele D. Facendola
		class DX11RenderTarget : public IRenderTarget{

		public:

			/// \brief Create a multiple render target array.
			DX11RenderTarget(const IRenderTarget::FromDescription& args);

			/// \brief Create a render target from a render target view.
			DX11RenderTarget(const COMPtr<ID3D11RenderTargetView>& render_target_view);

			virtual ~DX11RenderTarget();

			virtual size_t GetSize() const override;

			virtual size_t GetCount() const override;

			virtual ObjectPtr<ITexture2D> operator[](size_t index) override;

			virtual ObjectPtr<const ITexture2D> operator[](size_t index) const override;

			virtual ObjectPtr<ITexture2D> GetDepthBuffer() override;

			virtual ObjectPtr<const ITexture2D> GetDepthBuffer() const override;

			virtual bool Resize(unsigned int width, unsigned int height) override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;
			
			virtual vector<TextureFormat> GetFormat() const override;

			/// \brief Clear the depth stencil view.
			/// \param context The context used to clear the view.
			/// \param clear_flags Determines whether to clear the depth and\or the stencil buffer. (see: D3D11_CLEAR_FLAGS)
			/// \param depth Depth value to store inside the depth buffer.
			/// \param stencil Stencil value to store inside the stencil buffer.
			void ClearDepth(ID3D11DeviceContext& context, unsigned int clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, float depth = 1.0f, unsigned char stencil = 0);

			/// \brief Clear every target view.
			/// \param context The context used to clear the view.
			/// \param color The color used to clear the targets.
			void ClearTargets(ID3D11DeviceContext& context, Color color = kTransparentBlack);

			/// \brief Bind the render target to the given render context.
            /// \param depth_only Whether to bind the Z buffer only or not
			void Bind(ID3D11DeviceContext& context, bool depth_only = false);

			/// \brief Bind the render target and the given unordered access view to a render context.
            void Bind(ID3D11DeviceContext& context, const vector<ID3D11UnorderedAccessView*>& uav_list, bool depth_only = false);
			
			/// \brief Unbind the render target from the given render context.
			void Unbind(ID3D11DeviceContext& context);
			
			/// \brief Unbind the render target from the given render context.
			void Unbind(ID3D11DeviceContext& context, const vector<ID3D11UnorderedAccessView*>& uav_list);

		private:

			/// \brief Create the render target surfaces.
			/// The method will allocate one texture for each specified format. The dimensions are the same for each surface.
			/// \param width The width of each texture.
			/// \param height The height of each texture.
			/// \param target_format The format of each texture.
			void CreateSurfaces(unsigned int width, unsigned int height, const std::vector<TextureFormat>& target_format, bool depth);

			std::vector<ObjectPtr<DX11RenderTexture2D>> render_target_;			///< \brief Render target surfaces.

			ObjectPtr<DX11DepthTexture2D> depth_stencil_;						///< \brief Depth surface.

			D3D11_VIEWPORT viewport_;											///< \brief Render target viewport.

		};
		
		/// \brief Render-target cache under DirectX11.
		/// \author Raffaele D. Facendola.
		class DX11RenderTargetCache : public IRenderTargetCache {

		public:

			DX11RenderTargetCache(const Singleton&);

			virtual void PushToCache(const ObjectPtr<IRenderTarget>& texture) override;

			virtual ObjectPtr<IRenderTarget> PopFromCache(unsigned int width, unsigned int height, std::vector<TextureFormat> format, bool has_depth, bool generate = true) override;

			virtual size_t GetSize() const override;

			static void PurgeCache();

		private:
			
			static std::vector<ObjectPtr<DX11RenderTarget>> cache_;				///<\ brief Orphaned resource cache.

		};

		/// \brief DirectX11 render target array.
		/// \author Raffaele D. Facendola
		class DX11RenderTargetArray : public IRenderTargetArray {

		public:

			/// \brief Create a render target array.
			DX11RenderTargetArray(const IRenderTargetArray::FromDescription& args);

			virtual ~DX11RenderTargetArray();

			virtual size_t GetCount() const override;

			virtual ObjectPtr<ITexture2DArray> GetRenderTargets() override;

			virtual ObjectPtr<const ITexture2DArray> GetRenderTargets() const override;

			virtual ObjectPtr<ITexture2D> GetDepthBuffer() override;

			virtual ObjectPtr<const ITexture2D> GetDepthBuffer() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual size_t GetSize() const override;

			/// \brief Clear the depth stencil view.
			/// \param context The context used to clear the view.
			/// \param clear_flags Determines whether to clear the depth and\or the stencil buffer. (see: D3D11_CLEAR_FLAGS)
			/// \param depth Depth value to store inside the depth buffer.
			/// \param stencil Stencil value to store inside the stencil buffer.
			void ClearDepth(ID3D11DeviceContext& context, unsigned int clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, float depth = 1.0f, unsigned char stencil = 0);

			/// \brief Clear every target view.
			/// \param context The context used to clear the view.
			/// \param color The color used to clear the targets.
			void ClearTargets(ID3D11DeviceContext& context, Color color = kTransparentBlack);

			/// \brief Bind an element of the array to the given render context.
			/// \param index Index of the element to bind.
			/// \param viewport Viewport to use. Set this parameter to null to render to the whole surface.
			void Bind(ID3D11DeviceContext& context, unsigned int index, D3D11_VIEWPORT* viewport = nullptr);

			/// \brief Unbind the render target from the given render context.
			void Unbind(ID3D11DeviceContext& context);

		private:

			ObjectPtr<DX11Texture2DArray> render_target_array_;					///< \brief Actual render target array surfaces.

			ObjectPtr<DX11DepthTexture2D> depth_stencil_;						///< \brief Depth surface.

			vector<COMPtr<ID3D11RenderTargetView>> rtv_list_;					///< \brief List containing the render target view for each array element.

			D3D11_VIEWPORT viewport_;											///< \brief Render target viewport.

		};

		/////////////////////////////// DX11 DEPTH TEXTURE 2D ///////////////////////////////

		inline COMPtr<ID3D11DepthStencilView> DX11DepthTexture2D::GetDepthStencilView() const{

			return depth_stencil_view_;

		}

		/////////////////////////////// DX11 RENDER TEXTURE 2D ///////////////////////////////

		inline COMPtr<ID3D11RenderTargetView> DX11RenderTexture2D::GetRenderTargetView() const{

			return render_target_view_;

		}

		/////////////////////////////// DX11 RENDER TARGET ///////////////////////////////

		inline size_t DX11RenderTarget::GetSize() const{

			return std::accumulate(render_target_.begin(),
								   render_target_.end(),
								   depth_stencil_->GetSize(),
								   [](size_t size, const ObjectPtr<DX11RenderTexture2D>& texture){

										return size + texture->GetSize();

								   });

		}

		inline size_t DX11RenderTarget::GetCount() const 
		{

			return render_target_.size();

		}

		inline ObjectPtr<ITexture2D> DX11RenderTarget::operator[](size_t index){

			return ObjectPtr<ITexture2D>(render_target_[index]);

		}

		inline ObjectPtr<const ITexture2D> DX11RenderTarget::operator[](size_t index) const{

			return ObjectPtr<const ITexture2D>(render_target_[index]);

		}

		inline ObjectPtr<ITexture2D> DX11RenderTarget::GetDepthBuffer(){

			return ObjectPtr<ITexture2D>(depth_stencil_);

		}

		inline ObjectPtr<const ITexture2D> DX11RenderTarget::GetDepthBuffer() const{

			return ObjectPtr<const ITexture2D>(depth_stencil_);

		}

		inline unsigned int DX11RenderTarget::GetWidth() const{

			return render_target_.size() > 0 ?
				   render_target_[0]->GetWidth() :
				   depth_stencil_->GetWidth();

		}

		inline unsigned int DX11RenderTarget::GetHeight() const{

			return render_target_.size() > 0 ?
				   render_target_[0]->GetHeight() :
				   depth_stencil_->GetHeight();

		}

		///////////////////////////// DX11 RENDER TARGET CACHE //////////////////////////////

		INSTANTIABLE(IRenderTargetCache, DX11RenderTargetCache, IRenderTargetCache::Singleton);

		/////////////////////////////// DX11 RENDER TARGET ARRAY //////////////////////////////////////

		inline ObjectPtr<ITexture2DArray> DX11RenderTargetArray::GetRenderTargets() {

			return ObjectPtr<ITexture2DArray>(render_target_array_);

		}

		inline ObjectPtr<const ITexture2DArray> DX11RenderTargetArray::GetRenderTargets() const {

			return ObjectPtr<const ITexture2DArray>(render_target_array_);

		}

		inline ObjectPtr<ITexture2D> DX11RenderTargetArray::GetDepthBuffer() {

			return ObjectPtr<ITexture2D>(depth_stencil_);

		}

		inline ObjectPtr<const ITexture2D> DX11RenderTargetArray::GetDepthBuffer() const {

			return ObjectPtr<const ITexture2D>(depth_stencil_);

		}

		inline unsigned int DX11RenderTargetArray::GetWidth() const {

			return render_target_array_->GetHeight();
		}

		inline unsigned int DX11RenderTargetArray::GetHeight() const {

			return render_target_array_->GetHeight();

		}

		inline size_t DX11RenderTargetArray::GetCount() const {

			return render_target_array_->GetCount();

		}

		inline size_t DX11RenderTargetArray::GetSize() const {

			return render_target_array_->GetSize() +
				   depth_stencil_->GetSize();

		}

		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11RenderTarget> resource_cast(const ObjectPtr<IRenderTarget>& resource){

			return ObjectPtr<DX11RenderTarget>(resource.Get());

		}

	}

}

#endif