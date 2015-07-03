/// \file dx11texture.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "texture.h"

#include "dx11/dx11.h"
#include "dx11/dx11resources.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

		/// \brief DirectX11 2D texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public ITexture2D{

		public:

			/// \brief Create a new texture from DDS file.
			/// \param device The device used to create the texture.
			/// \param bundle The bundle used to load the texture.
			DX11Texture2D(const FromFile& args);

			/// \brief Create a new texture.
			/// \param texture The texture to bind.
			/// \param shader_view The view used to bind the texture to the shader.
			DX11Texture2D(ID3D11Texture2D& texture, ID3D11ShaderResourceView& shader_view);

			/// \brief Create a new texture from an existing DirectX11 texture.
			/// \param texture The DirectX11 texture.
			/// \param format The format used when sampling from the texture.
			DX11Texture2D(ID3D11Texture2D& texture, DXGI_FORMAT format);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMipCount() const override;

			DXGI_FORMAT GetFormat() const;

			/// \brief Get the shader resource view used to bind this texture to the pipeline.
			ID3D11ShaderResourceView* GetShaderResourceView() const;

		private:

			void UpdateDescription();

			unique_com<ID3D11Texture2D> texture_;							///< \brief Pointer to the actual texture.

			unique_com<ID3D11ShaderResourceView> shader_view_;				///< \brief Pointer to the shader resource view of the texture.

			unsigned int width_;											///< \brief Width of the texture, in pixels.

			unsigned int height_;											///< \brief Height of the texture, in pixels.
			
			unsigned int bits_per_pixel_;									///< \brief Bits per pixel.

			unsigned int mip_levels_;										///< \brief Mip levels.

			DXGI_FORMAT format_;											///< \brief Surface format.

		};

	}

}

/////////////////////////////// DX11 TEXTURE2D ///////////////////////////////

inline unsigned int gi_lib::dx11::DX11Texture2D::GetWidth() const{

	return width_;

}

inline unsigned int gi_lib::dx11::DX11Texture2D::GetHeight()const {

	return height_;

}

inline unsigned int gi_lib::dx11::DX11Texture2D::GetMipCount() const{

	return mip_levels_;

}

inline DXGI_FORMAT gi_lib::dx11::DX11Texture2D::GetFormat() const{

	return format_;

}

#endif