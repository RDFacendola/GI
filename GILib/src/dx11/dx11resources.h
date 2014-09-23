/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <d3dx11effect.h>
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

			virtual size_t GetMipMapCount() const override;

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

			size_t mip_levels_;
			
			WrapMode wrap_mode_;

		};

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh: public Mesh{

		public:

			DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kNormalTextured> & settings);

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual size_t GetVertexCount() const override;

			virtual size_t GetPolygonCount() const override;

			virtual size_t GetLODCount() const override;

		private:

			unique_ptr<ID3D11Buffer, COMDeleter> vertex_buffer_;

			unique_ptr<ID3D11Buffer, COMDeleter> index_buffer_;

			size_t vertex_count_;

			size_t polygon_count_;

			size_t LOD_count_;

			size_t size_;
			
		};

		/// \brief DirectX11 shader.
		/// \author Raffaele D. Facendola.
		class DX11Shader : public Shader{

		public:

			DX11Shader(ID3D11Device & device, const LoadSettings<Shader, Shader::LoadMode::kCompileFromFile> & settings);

			/// \brief No copy constructor.
			DX11Shader(DX11Shader &) = delete;

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			/// \brief Get the shader effect.
			/// \return Returns a reference to the shader effect.
			ID3DX11Effect & GetEffect();

			/// \brief Get the shader effect.
			/// \return Returns a reference to the shader effect.
			const ID3DX11Effect & GetEffect() const;

			/// \brief Clone the effect inside another effect.
			void CloneEffect(ID3DX11Effect ** effect) const;

		private:

			unique_ptr<ID3DX11Effect, COMDeleter> effect_;

			ResourcePriority priority_;

			size_t size_;

		};

		/// \brief DirectX11 material.
		/// \author Raffaele D. Facendola
		class DX11Material : public Material{

		public:

			DX11Material(ID3D11Device & device, const BuildSettings<Material, Material::BuildMode::kFromShader> & settings);

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual shared_ptr<MaterialParameter> GetParameterByName(const string & name) override;

			virtual shared_ptr<MaterialParameter> GetParameterBySemantic(const string & semantic) override;

			virtual Shader & GetShader() override;

			virtual const Shader & GetShader() const override;

		private:

			shared_ptr<Shader> shader_;

			unique_ptr<ID3DX11Effect, COMDeleter> effect_;

			ResourcePriority priority_;

			size_t size_;

		};

		/// \brief Base interface for material parameters.

		/// \author Raffaele D. Facendola
		class DX11MaterialParameter: public MaterialParameter{

		public:

			/// \brief Create a new material parameter.
			/// \param variable The variable accessed by this parameter
			DX11MaterialParameter(shared_ptr<ID3DX11EffectVariable> variable);

			virtual ~DX11MaterialParameter();

			virtual bool Read(bool & out);

			virtual bool Read(float & out);

			virtual bool Read(int & out);

			virtual bool Read(Vector2f & out);

			virtual bool Read(Vector3f & out);

			virtual bool Read(Vector4f & out);

			virtual bool Read(Affine3f & out);

			virtual bool Read(Projective3f & out);

			virtual bool Read(shared_ptr<Texture2D> & out);

			virtual bool Read(void ** out);

			virtual bool Write(const bool & in);

			virtual bool Write(const float & in);

			virtual bool Write(const int & in);

			virtual bool Write(const Vector2f & in);

			virtual bool Write(const Vector3f & in);

			virtual bool Write(const Vector4f & in);

			virtual bool Write(const Affine3f & in);

			virtual bool Write(const Projective3f & in);

			virtual bool Write(const shared_ptr<Texture2D> in);

			virtual bool Write(void ** in);

		private:

			shared_ptr<ID3DX11EffectVariable> variable_;

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

		/// \brief Shader mapping.
		template<> struct ResourceMapping < Shader > {

			/// \brief Concrete type associated to a Shader.
			using TMapped = DX11Shader;

		};

		/// \brief Material mapping.
		template<> struct ResourceMapping < Material > {

			/// \brief Concrete type associated to a Material.
			using TMapped = DX11Material;

		};

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource>
		typename ResourceMapping<TResource>::TMapped & resource_cast(shared_ptr<TResource> & resource){

			return *static_cast<typename ResourceMapping<TResource>::TMapped *>(resource.get());

		}

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource>
		const typename ResourceMapping<TResource>::TMapped & resource_cast(const shared_ptr<TResource> & resource){

			return *static_cast<typename ResourceMapping<TResource>::TMapped *>(resource.get());

		}

		//

		inline size_t DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline size_t DX11Texture2D::GetHeight()const {

			return height_;

		}

		inline size_t DX11Texture2D::GetMipMapCount() const{

			return mip_levels_;

		}

		inline WrapMode DX11Texture2D::GetWrapMode() const{

			return wrap_mode_;

		}

		inline void DX11Texture2D::SetWrapMode(WrapMode wrap_mode){

			wrap_mode_ = wrap_mode;

		}

		//

		inline size_t DX11Mesh::GetVertexCount() const{

			return vertex_count_;

		}

		inline size_t DX11Mesh::GetPolygonCount() const{

			return polygon_count_;

		}

		inline size_t DX11Mesh::GetLODCount() const{

			return LOD_count_;

		}

		inline size_t DX11Mesh::GetSize() const{

			return size_;

		}

		//

		inline size_t DX11Shader::GetSize() const{

			return size_;

		}

		inline ResourcePriority DX11Shader::GetPriority() const{

			return priority_;

		}

		inline void DX11Shader::SetPriority(ResourcePriority priority){

			priority_ = priority;

		}

		inline ID3DX11Effect & DX11Shader::GetEffect(){

			return *effect_.get();

		}

		inline const ID3DX11Effect & DX11Shader::GetEffect() const{

			return *effect_.get();

		}

		//

		inline size_t DX11Material::GetSize() const{

			return size_;

		}

		inline ResourcePriority DX11Material::GetPriority() const{

			return priority_;

		}

		inline void DX11Material::SetPriority(ResourcePriority priority){

			priority_ = priority;

		}

		inline Shader & DX11Material::GetShader() {

			return *shader_;

		}

		inline const Shader & DX11Material::GetShader() const {

			return *shader_;

		}

	}

}