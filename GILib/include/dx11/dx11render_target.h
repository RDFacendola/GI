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
			/// \param width Width of each target.
			/// \param height Height of each target.
			/// \param target_format Describe the format of each target.
			DX11RenderTarget(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format);

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
			void Bind(ID3D11DeviceContext& context);

			/// \brief Unbind the render target from the given render context.
			void Unbind(ID3D11DeviceContext& context);

		private:

			/// \brief Create the render target surfaces.
			/// The method will allocate one texture for each specified format. The dimensions are the same for each surface.
			/// \param width The width of each texture.
			/// \param height The height of each texture.
			/// \param target_format The format of each texture.
			void CreateSurfaces(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format);

			std::vector<ObjectPtr<DX11RenderTexture2D>> render_target_;			///< \brief Render target surfaces.

			ObjectPtr<DX11DepthTexture2D> depth_stencil_;						///< \brief Depth surface.

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

			return render_target_[0]->GetWidth();

		}

		inline unsigned int DX11RenderTarget::GetHeight() const{

			return render_target_[0]->GetHeight();

		}

		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11RenderTarget> resource_cast(const ObjectPtr<IRenderTarget>& resource){

			return ObjectPtr<DX11RenderTarget>(resource.Get());

		}

	}

}

#endif