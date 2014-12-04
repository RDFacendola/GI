/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <d3dx11effect.h>
#include <string>
#include <map>
#include <memory>
#include <numeric>

#include "..\..\include\resources.h"
#include "..\..\include\resource_traits.h"
#include "dx11shared.h"

using ::std::wstring;
using ::std::unique_ptr;
using ::std::shared_ptr;
using ::std::map;

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

			/// \brief Create a mew texture from an existing DirectX11 texture.
			/// \param texture The DirectX11 texture.
			DX11Texture2D(ID3D11Texture2D & texture);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMipMapCount() const override;

			virtual WrapMode GetWrapMode() const override;

			virtual void SetWrapMode(WrapMode wrap_mode) override;

			/// \brief Get the view used to bind this texture to a shader.
			/// \return Returns a reference to the shader resource view.
			ID3D11ShaderResourceView & GetShaderResourceView();

			/// \brief Get the view used to bind this texture to a shader.
			/// \return Returns a reference to the shader resource view.
			const ID3D11ShaderResourceView & GetShaderResourceView() const;

		private:

			void UpdateDescription();

			unique_ptr<ID3D11Texture2D, COMDeleter> texture_;

			unique_ptr<ID3D11ShaderResourceView, COMDeleter> shader_view_;

			unsigned int width_;

			unsigned int height_;

			unsigned int bits_per_pixel_;

			unsigned int mip_levels_;
			
			WrapMode wrap_mode_;

		};

		/// \brief DirectX11 render target.
		/// \author Raffaele D. Facendola
		class DX11RenderTarget : public RenderTarget{

		public:

			/// \brief Create a new render target from an existing buffer.

			/// \param buffer Buffer reference.
			DX11RenderTarget(ID3D11Texture2D & buffer);

			virtual ~DX11RenderTarget(){}

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual unsigned int GetCount() const override;

			virtual shared_ptr<Texture2D> GetTexture(int index) override;

			virtual shared_ptr<const Texture2D> GetTexture(int index) const override;

			virtual float GetAspectRatio() const override;

			/// \brief Set new buffers for the render target.

			/// \param buffers The list of buffers to bound
			void SetBuffers(std::initializer_list<ID3D11Texture2D*> buffers);

			/// \brief Releases al the buffers referenced by the render target.
			void ResetBuffers();

			/// \brief Bind the render target to the specified context.
			/// \param context The context to bound the render target to.
			void Bind(ID3D11DeviceContext & context);

		private:
			
			unique_ptr < ID3D11DepthStencilView, COMDeleter > depth_stencil_view_;

			vector<unique_ptr<ID3D11RenderTargetView, COMDeleter>> target_views_;

			vector<shared_ptr<DX11Texture2D>> textures_;

		};

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh: public Mesh{

		public:

			/// \brief Create a new DirectX11 mesh.
			/// \param device The device used to load the graphical resources.
			/// \param settings Settings used to build the mesh.
			DX11Mesh(ID3D11Device & device, const BuildSettings<Mesh, Mesh::BuildMode::kNormalTextured> & settings);

			virtual size_t GetSize() const override;

			virtual ResourcePriority GetPriority() const override;

			virtual void SetPriority(ResourcePriority priority) override;

			virtual size_t GetVertexCount() const override;

			virtual size_t GetPolygonCount() const override;

			virtual size_t GetLODCount() const override;

			virtual Bounds GetBounds() const override;

		private:

			unique_ptr<ID3D11Buffer, COMDeleter> vertex_buffer_;

			unique_ptr<ID3D11Buffer, COMDeleter> index_buffer_;

			size_t vertex_count_;

			size_t polygon_count_;

			size_t LOD_count_;

			size_t size_;

			Bounds bounds_;
			
		};

		/// \brief DirectX11 shader.
		/// \author Raffaele D. Facendola.
		class DX11Shader : public Shader{

		public:

			/// \brief Create a new DirectX11 shader.
			/// \param device The device used to load the graphical resources.
			/// \param settings The settings used to build the shader.
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

			friend class DX11MaterialParameter;

		public:

			/// \brief Create a new DirectX11 material instance.
			/// \param device The device used to load the graphical resources.
			/// \param settings The settings used to build the material.
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

			map<string, shared_ptr<Resource>> resources_;		///< \brief Map of inner resources. This is used to prevent destruction while a shader is still using a particular resource.

			ResourcePriority priority_;

			size_t size_;

		};

		/// \brief Base interface for material parameters.

		/// \author Raffaele D. Facendola
		class DX11MaterialParameter: public MaterialParameter{

		public:

			/// \brief Create a new material parameter.
			/// \param variable The variable accessed by this parameter.
			/// \param material The material this parameter refers to.
			DX11MaterialParameter(shared_ptr<ID3DX11EffectVariable> variable, DX11Material & material);

			/// \brief No assignment operator.
			DX11MaterialParameter & operator=(const DX11MaterialParameter &) = delete;

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

			D3DX11_EFFECT_VARIABLE_DESC metadata_;
			
			DX11Material & material_;

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

		/// \brief Render target mapping.
		template<> struct ResourceMapping < RenderTarget > {

			/// \brief Concrete type associated to a Render Target
			using TMapped = DX11RenderTarget;

		};

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource>
		typename ResourceMapping<TResource>::TMapped & resource_cast(TResource & resource){

			return static_cast<typename ResourceMapping<TResource>::TMapped&>(resource);

		}

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource>
		typename const ResourceMapping<TResource>::TMapped & resource_cast(const TResource & resource){

			return static_cast<const typename ResourceMapping<TResource>::TMapped&>(resource);

		}


		//

		inline unsigned int DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline unsigned int DX11Texture2D::GetHeight()const {

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

		inline ID3D11ShaderResourceView & DX11Texture2D::GetShaderResourceView(){

			return *shader_view_;
		}

		inline const ID3D11ShaderResourceView & DX11Texture2D::GetShaderResourceView() const{

			return *shader_view_;

		}

		// DX11RenderTarget

		inline size_t DX11RenderTarget::GetSize() const{

			return std::accumulate(textures_.begin(), 
								   textures_.end(), 
								   static_cast<size_t>(0), 
								   [](size_t size, const shared_ptr<DX11Texture2D> texture){ 
				
										return size + texture->GetSize(); 
			
								   });
			
		}

		inline ResourcePriority DX11RenderTarget::GetPriority() const{

			return textures_[0]->GetPriority();

		}

		inline void DX11RenderTarget::SetPriority(ResourcePriority priority){

			for (auto & texture : textures_){

				texture->SetPriority(priority);

			}

		}

		inline float DX11RenderTarget::GetAspectRatio() const{

			// The aspect ratio is guaranteed to be the same for all the targets.
			return static_cast<float>(textures_[0]->GetWidth()) /
				static_cast<float>(textures_[0]->GetHeight());

		}

		inline unsigned int DX11RenderTarget::GetCount() const{

			return static_cast<unsigned int>(textures_.size());

		}

		inline shared_ptr<Texture2D> DX11RenderTarget::GetTexture(int index){

			return std::static_pointer_cast<Texture2D>(textures_[index]);

		}

		inline shared_ptr<const Texture2D> DX11RenderTarget::GetTexture(int index) const{

			return std::static_pointer_cast<const Texture2D>(textures_[index]);

		}

		// DX11Mesh

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

		inline Bounds DX11Mesh::GetBounds() const{

			return bounds_;

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