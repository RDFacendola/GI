/// \file dx11texture.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <string.h>
#include <memory>

#include "texture.h"

namespace gi_lib{

	namespace dx11{

		/// \brief DirectX11 plain texture.

		/// The texture can be bound to a shader and is immutable.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public Texture2D{

		public:
			
			/// \brief Create a new texture from file.
			/// param device The device used to create the texture.
			/// \param path The path of the texture to load.
			DX11Texture2D(ID3D11Device & device, const wstring & path);

			~DX11Texture2D();

			virtual size_t GetSize() const;

			virtual ResourcePriority GetPriority() const;

			virtual void SetPriority(ResourcePriority priority);

			virtual size_t GetWidth() const;

			virtual size_t GetHeight() const;

			virtual unsigned int GetMipMapCount() const;

			virtual WrapMode GetWrapMode() const;

			virtual void SetWrapMode(WrapMode wrap_mode);

		private:

			ID3D11Texture2D * texture_;

			// Used to bind the texture to the shader.
			ID3D11ShaderResourceView * view_;

			// This texture has an alpha channel.
			bool alpha_;

			size_t width_;

			size_t height_;

			size_t bits_per_pixel_;

			unsigned int mip_levels_;
			
			WrapMode wrap_mode_;

		};

		inline size_t DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline size_t DX11Texture2D::GetHeight()const {

			return height_;

		}

		inline unsigned int DX11Texture2D::GetMipMapCount() const{

			return mip_levels_;

		}

		inline WrapMode DX11Texture2D::GetWrapMode() const{

			return wrap_mode_;

		}

		inline void DX11Texture2D::SetWrapMode(WrapMode wrap_mode){

			wrap_mode_ = wrap_mode;

		}

	}

}