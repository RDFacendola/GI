/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <string>
#include <memory>

#include "resources.h"
#include "dx11shared.h"

using ::std::wstring;
using ::std::unique_ptr;
using ::std::shared_ptr;

namespace gi_lib{

	namespace dx11{

		class DX11Texture2D;

		/// \brief DirectX11 resource traits
		template<typename TResource> struct resource_traits;

		/// \brief DirectX11 resource traits
		template<> struct resource_traits < Texture2D > {

			/// \brief Concreate type associated to a Texture2D.
			using type = DX11Texture2D;

		};

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		typename shared_ptr<typename resource_traits<TResource>::type> resource_cast(shared_ptr<TResource> & resource);

		/// \brief DirectX11 plain texture.

		/// The texture can be bound to a shader and is immutable.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public Texture2D{

		public:
			
			/// \brief Create a new texture from file.
			/// \param device The device used to create the texture.
			/// \param path The path of the texture to load.
			DX11Texture2D(ID3D11Device & device, const wstring & path);

			virtual size_t GetSize() const;

			virtual ResourcePriority GetPriority() const;

			virtual void SetPriority(ResourcePriority priority);

			virtual size_t GetWidth() const;

			virtual size_t GetHeight() const;

			virtual unsigned int GetMipMapCount() const;

			virtual WrapMode GetWrapMode() const;

			virtual void SetWrapMode(WrapMode wrap_mode);

		private:

			unique_ptr<ID3D11Texture2D, COMDeleter> texture_;

			unique_ptr<ID3D11ShaderResourceView, COMDeleter> shader_view_;

			// This texture has an alpha channel.
			bool alpha_;

			size_t width_;

			size_t height_;

			size_t bits_per_pixel_;

			unsigned int mip_levels_;
			
			WrapMode wrap_mode_;

		};

		//

		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type*>
		typename shared_ptr<typename resource_traits<TResource>::type> resource_cast(shared_ptr<TResource> & resource){

			return static_pointer_cast<typename resource_traits<TResource>::type> (resource.operator->());

		}

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