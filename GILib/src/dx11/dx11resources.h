/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <string>
#include <memory>

#include "..\..\include\resources.h"
#include "..\..\include\resource_traits.h"
#include "dx11shared.h"

using ::std::wstring;
using ::std::unique_ptr;
using ::std::shared_ptr;

namespace gi_lib{

	namespace dx11{

		class DX11Texture2D;
		class DX11Mesh;

		/// \brief DirectX11 plain texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public Texture2D{

		public:
			
			/// \brief Create a new texture from DDS file.
			/// \param device The device used to create the texture.
			/// \param settings The load settings
			DX11Texture2D(ID3D11Device & device, const LoadSettings<Texture2D, Texture2D::LoadMode::kFromDDS> & settings);

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual size_t GetWidth() const override;

			virtual size_t GetHeight() const override;

			virtual unsigned int GetMipMapCount() const override;

			virtual WrapMode GetWrapMode() const override;

			virtual void SetWrapMode(WrapMode wrap_mode) override;

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

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh: public Mesh{

		public:

			DX11Mesh(ID3D11Device & device, const LoadSettings<Mesh, Mesh::LoadMode::kFromFBX> & settings);

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual unsigned int GetVertexCount() const override;

			virtual unsigned int GetPolygonCount() const override;

			virtual unsigned int GetLODCount() const override;

		private:

			unique_ptr<ID3D11Buffer, COMDeleter> vertex_buffer;

			unique_ptr<ID3D11Buffer, COMDeleter> index_buffer;

			unsigned int vertex_count_;

			unsigned int polygon_count_;

			unsigned int LOD_count_;

		};

		/// \brief DirectX11 resource mapping template.
		template<typename TResource> struct ResourceMapping;

		/// \brief Texture 2D mapping
		template<> struct ResourceMapping < Texture2D > {

			/// \brief Concrete type associated to a Texture2D.
			using TMapped = DX11Texture2D;

		};

		/// \brief Mesh mapping.
		template<> struct ResourceMapping < Mesh > {

			/// \brief Concrete type associated to a Mesh.
			using TMapped = DX11Mesh;

		};

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type*>
		typename shared_ptr<typename ResourceMapping<TResource>::TMapped> resource_cast(shared_ptr<TResource> & resource){

			return static_pointer_cast<typename ResourceMapping<TResource>::TMapped> (resource.operator->());

		}

		//

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

		//

		inline unsigned int DX11Mesh::GetVertexCount() const{

			return vertex_count_;

		}

		inline unsigned int DX11Mesh::GetPolygonCount() const{

			return polygon_count_;

		}

		inline unsigned int DX11Mesh::GetLODCount() const{

			return LOD_count_;

		}


	}

}