/// \file dx11texture.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "texture.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;

		/// \brief DirectX11 2D texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public ITexture2D{

		public:

			/// \brief Create a new texture from DDS file.
			/// \param device The device used to create the texture.
			/// \param bundle The bundle used to load the texture.
			DX11Texture2D(const FromFile& args);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMIPCount() const override;

			DXGI_FORMAT GetFormat() const;

			/// \brief Get the shader resource view used to bind this texture to the pipeline.
			COMPtr<ID3D11ShaderResourceView> GetShaderResourceView() const;

		protected:

			/// \brief Create an empty texture.
			DX11Texture2D();

			/// \brief Initialize the texture from a shader resource view.
			/// You may initialize the same texture multiple times to recycle an unused object.
			void Initialize(COMPtr<ID3D11ShaderResourceView> shader_resource_view);

		private:

			void UpdateDescription(const D3D11_TEXTURE2D_DESC& description);

			COMPtr<ID3D11ShaderResourceView> shader_resource_view_;		///< \brief Pointer to the shader resource view of the texture.

			unsigned int width_;										///< \brief Width of the texture, in pixels.

			unsigned int height_;										///< \brief Height of the texture, in pixels.
			
			unsigned int bits_per_pixel_;								///< \brief Bits per pixel.

			unsigned int mip_levels_;									///< \brief MIP levels.

			DXGI_FORMAT format_;										///< \brief Surface format.

		};

		class DX11GPTexture2D : public IGPTexture2D{



		};

		/////////////////////////////// DX11 TEXTURE2D ///////////////////////////////

		inline unsigned int DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline unsigned int DX11Texture2D::GetHeight()const {

			return height_;

		}

		inline unsigned int DX11Texture2D::GetMIPCount() const{

			return mip_levels_;

		}

		inline DXGI_FORMAT DX11Texture2D::GetFormat() const{

			return format_;

		}

		inline COMPtr<ID3D11ShaderResourceView> DX11Texture2D::GetShaderResourceView() const{

			return shader_resource_view_;

		}

	}

}

#endif