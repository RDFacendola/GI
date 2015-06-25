/// \file dx11render_target.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "render_target.h"

#include "dx11/dx11.h"
#include "dx11/dx11resources.h"
#include "dx11/dx11texture.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		/// \brief DirectX11 render target.
		/// \author Raffaele D. Facendola
		class DX11RenderTarget : public IRenderTarget{

		public:

			/// \brief Create a new render target from an existing buffer.
			/// \param buffer Buffer reference.
			/// \param device Device used to create the additional internal resources.
			DX11RenderTarget(ID3D11Texture2D& target);

			/// \brief Create a multiple render target array.
			/// \param width Width of each target.
			/// \param height Height of each target.
			/// \param target_format Describe the format of each target.
			/// \param unordered_access Whether the render target shall be bound as unordered access for a compute shader.
			DX11RenderTarget(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format, bool unordered_access = false);

			virtual ~DX11RenderTarget();

			virtual size_t GetSize() const override;

			virtual unsigned int GetCount() const override;

			virtual ObjectPtr<ITexture2D> operator[](size_t index) override;

			virtual ObjectPtr<const ITexture2D> operator[](size_t index) const override;

			virtual ObjectPtr<ITexture2D> GetZStencil() override;

			virtual ObjectPtr<const ITexture2D> GetZStencil() const override;

			virtual float GetAspectRatio() const override;

			//virtual AntialiasingMode GetAntialiasing() const override;

			virtual bool Resize(unsigned int width, unsigned int height) override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			/// \brief Set new buffers for the render target.

			/// \param buffers The list of buffers to bound
			void SetBuffers(std::initializer_list<ID3D11Texture2D*> targets);

			/// \brief Releases all the buffers referenced by the render target.
			void ResetBuffers();

			/// \brief Clear the depth stencil view.
			/// \param context The context used to clear the view.
			/// \param clear_flags Determines whether to clear the depth and\or the stencil buffer. (see: D3D11_CLEAR_FLAGS)
			/// \param depth Depth value to store inside the depth buffer.
			/// \param stencil Stencil value to store inside the stencil buffer.
			void ClearDepthStencil(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil);

			/// \brief Clear every target view.
			/// \param context The context used to clear the view.
			/// \param color The color used to clear the targets.
			void ClearTargets(ID3D11DeviceContext& context, Color color);

			/// \brief Bind the render target to the given render context.
			void Bind(ID3D11DeviceContext& context);

		private:

			/// \brief Initialize the GBuffer surfaces.
			/// The method will allocate one texture for each specified format. The dimensions are the same for each surface.
			/// \param width The width of each texture.
			/// \param height The height of each texture.
			/// \param target_format The format of each texture.
			void Initialize(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format);

			vector<ObjectPtr<DX11Texture2D>> textures_;				///< \brief Render target surfaces.

			vector<ID3D11RenderTargetView*> target_views_;			///< \brief Render target view of each target surface.

			ObjectPtr<DX11Texture2D> zstencil_;						///< \brief ZStencil surface.

			ID3D11DepthStencilView* zstencil_view_;					///< \brief Depth stencil view of the ZStencil surface.

			D3D11_VIEWPORT viewport_;								///< \brief Render target viewport.

			bool unordered_access_;									///< \brief Whether the render targets can be bound also as UAV.

			AntialiasingMode antialiasing_;							///< \brief Antialiasing description. TODO: remove?

		};

	}

}

/////////////////////////////// DX11 RENDER TARGET ///////////////////////////////

inline size_t gi_lib::dx11::DX11RenderTarget::GetSize() const{

	return std::accumulate(textures_.begin(),
						   textures_.end(),
						   static_cast<size_t>(0),
						   [](size_t size, const ObjectPtr<DX11Texture2D>& texture){

								return size + texture->GetSize();

						   });

}

inline float gi_lib::dx11::DX11RenderTarget::GetAspectRatio() const{

	// The aspect ratio is guaranteed to be the same for all the targets.
	return static_cast<float>(GetWidth()) /
		   static_cast<float>(GetHeight());

}

//inline gi_lib::AntialiasingMode gi_lib::dx11::DX11RenderTarget::GetAntialiasing() const{
//
//	return antialiasing_;
//
//}

inline unsigned int gi_lib::dx11::DX11RenderTarget::GetCount() const{

	return static_cast<unsigned int>(textures_.size());

}

inline gi_lib::ObjectPtr<gi_lib::ITexture2D> gi_lib::dx11::DX11RenderTarget::operator[](size_t index){

	return textures_[index];

}

inline gi_lib::ObjectPtr<const gi_lib::ITexture2D> gi_lib::dx11::DX11RenderTarget::operator[](size_t index) const{

	return textures_[index];

}

inline gi_lib::ObjectPtr<gi_lib::ITexture2D> gi_lib::dx11::DX11RenderTarget::GetZStencil(){

	return zstencil_;

}

inline gi_lib::ObjectPtr<const gi_lib::ITexture2D> gi_lib::dx11::DX11RenderTarget::GetZStencil() const{

	return zstencil_;

}

inline unsigned int gi_lib::dx11::DX11RenderTarget::GetWidth() const{

	return textures_[0]->GetWidth();

}

inline unsigned int gi_lib::dx11::DX11RenderTarget::GetHeight() const{

	return textures_[0]->GetHeight();

}

#endif