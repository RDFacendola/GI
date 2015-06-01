/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <string>
#include <map>
#include <memory>
#include <numeric>

#include "..\..\include\graphics.h"
#include "..\..\include\resources.h"
#include "..\..\include\gilib.h"
#include "..\..\include\windows\win_os.h"

using ::std::string;
using ::std::wstring;
using ::std::unique_ptr;
using ::std::shared_ptr;
using ::std::map;

using ::gi_lib::windows::COMDeleter;

namespace gi_lib{

	namespace dx11{

		class DX11Texture2D;
		class DX11Mesh;
		
		/// \brief Base interface for DirectX11 resources that can be bound as shader resources.
		/// \author Raffaele D. Facendola
		class DX11ResourceView : public IResourceView {

		public:

			/// \brief Virtual destructor.
			virtual ~DX11ResourceView(){};

			/// \brief Get the shader resource view associated to this shader resource.
			/// \return Returns the shader resource view associated to this shader resource.
			virtual ID3D11ShaderResourceView& GetShaderView() = 0;

		};

		/// \brief Concrete templated resource view class.
		/// This class will hold a strong reference to a resource (so it is is not released while the view is still being used somewhere)
		/// and its shader resource view.
		template <typename TResource>
		class DX11ResourceViewTemplate : public DX11ResourceView{

		public:

			/// \brief Create a new resource view from a concrete resource type.
			/// \param resource Resource associated to the view.
			/// \param resource_view Shared resource view relative to the specified resource.
			DX11ResourceViewTemplate(ObjectPtr<TResource> resource, ID3D11ShaderResourceView& resource_view);

			/// \brief Virtual destructor.
			virtual ~DX11ResourceViewTemplate();

			/// \brief Get the resource's shader view.
			/// \return Returns the resource's shader view.
			virtual ID3D11ShaderResourceView& GetShaderView() override;
			
		private:

			ObjectPtr<TResource> resource_;				///< \brief Strong reference to the resource.

			ID3D11ShaderResourceView& resource_view_;	///< \brief Shader resource view.

		};

		/// \brief DirectX11 plain texture.
		/// \author Raffaele D. Facendola.
		class DX11Texture2D : public Texture2D{

		public:
			
			/// \brief Create a new texture from DDS file.
			/// \param device The device used to create the texture.
			/// \param bundle The bundle used to load the texture.
			DX11Texture2D(const FromFile& args);

			/// \brief Create a mew texture from an existing DirectX11 texture.
			/// \param texture The DirectX11 texture.
			/// \param format The format used when sampling from the texture.
			DX11Texture2D(ID3D11Texture2D& texture, DXGI_FORMAT format);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMipMapCount() const override;

			virtual ObjectPtr<IResourceView> GetView() override;

		private:

			void UpdateDescription();
					
			unique_ptr<ID3D11Texture2D, COMDeleter> texture_;

			unique_ptr<ID3D11ShaderResourceView, COMDeleter> shader_view_;

			unsigned int width_;

			unsigned int height_;

			unsigned int bits_per_pixel_;

			unsigned int mip_levels_;
		
		};

		/// \brief DirectX11 render target.
		/// \author Raffaele D. Facendola
		class DX11RenderTarget : public RenderTarget{

		public:

			/// \brief Create a new render target from an existing buffer.

			/// \param buffer Buffer reference.
			/// \param device Device used to create the additional internal resources.
			DX11RenderTarget(ID3D11Texture2D& target);

			virtual ~DX11RenderTarget();

			virtual size_t GetSize() const override;

			virtual unsigned int GetCount() const override;

			virtual ObjectPtr<Texture2D> GetTexture(int index) override;

			virtual ObjectPtr<const Texture2D> GetTexture(int index) const override;

			virtual ObjectPtr<Texture2D> GetZStencil() override;

			virtual ObjectPtr<const Texture2D> GetZStencil() const override;

			virtual float GetAspectRatio() const override;

			virtual AntialiasingMode GetAntialiasing() const override;

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
			
			vector<ID3D11RenderTargetView*> target_views_;
			
			ID3D11DepthStencilView* zstencil_view_;

			vector<ObjectPtr<DX11Texture2D>> textures_;

			ObjectPtr<DX11Texture2D> zstencil_;

			AntialiasingMode antialiasing_;

		};

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh: public Mesh{

		public:

			/// \brief Create a new DirectX11 mesh.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to create the mesh.
			DX11Mesh(const FromVertices<VertexFormatNormalTextured>& args);

			virtual size_t GetSize() const override;

			virtual size_t GetVertexCount() const override;

			virtual size_t GetPolygonCount() const override;

			virtual size_t GetLODCount() const override;

			virtual const AABB& GetBoundingBox() const override;

			virtual size_t GetSubsetCount() const override;

			virtual const MeshSubset& GetSubset(unsigned int subset_index) const override;

			/// \brief Bind the mesh to the given context.
			void Bind(ID3D11DeviceContext& context);

		private:

			unique_ptr<ID3D11Buffer, COMDeleter> vertex_buffer_;

			unique_ptr<ID3D11Buffer, COMDeleter> index_buffer_;

			vector<MeshSubset> subsets_;

			size_t vertex_count_;

			size_t polygon_count_;

			size_t LOD_count_;

			size_t size_;

			size_t vertex_stride_;											///< \brief Size of each vertex in bytes

			AABB bounding_box_;
			
		};

		/// \brief DirectX11 material.
		/// \author Raffaele D. Facendola
		class DX11Material : public Material{

		public:

			/// \brief Create a new DirectX11 material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11Material(const CompileFromFile& args);

			/// \brief Instantiate a DirectX11 material from another one.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to instantiate the material.
			DX11Material(const Instantiate& args);
			
			/// \brief Default destructor.
			~DX11Material();

			virtual size_t GetSize() const override;

			virtual ObjectPtr<MaterialVariable> GetVariable(const string& name) override;

			virtual ObjectPtr<MaterialResource> GetResource(const string& name) override;

			/// \brief Commit all the constant buffers and bind the material to the pipeline.
			void Commit(ID3D11DeviceContext& context);

		private:

			/// \brief Holds the properties shared among material instances.
			struct MaterialImpl;

			/// \brief Holds the private properties of this material instance.
			struct InstanceImpl;

			/// \brief Material variable.
			class DX11MaterialVariable : public MaterialVariable{

			public:
				
				DX11MaterialVariable(InstanceImpl& instance_impl, size_t buffer_index, size_t variable_size, size_t variable_offset);

			protected:
								
				virtual void Set(const void * buffer, size_t size) override;

			private:

				InstanceImpl* instance_impl_;

				size_t buffer_index_;

				size_t variable_offset_;		///< \brief Offset of the variable from the beginning of the buffer in bytes.

				size_t variable_size_;			///< \brief Size of the variable in bytes.

			};

			/// \brief Material resource.
			class DX11MaterialResource : public MaterialResource{

			public:

				DX11MaterialResource(InstanceImpl& instance_impl, size_t resource_index);

				virtual void Set(ObjectPtr<IResourceView> resource) override;

			private:

				InstanceImpl* instance_impl_;

				size_t resource_index_;

			};
			
			shared_ptr<MaterialImpl> shared_impl_;		///< \brief Properties shared among material instances.

			unique_ptr<InstanceImpl> private_impl_;		///< \brief Private properties of this material instance.
			
		};

		/// \brief DirectX11 resource mapping template.
		template<typename TResource> struct ResourceMapping;

		/// \brief Shader resource mapping.
		template<> struct ResourceMapping < IResourceView > {

			/// \brief Concrete type associated to a shader resource.
			using TMapped = DX11ResourceView;

		};

		/// \brief Texture 2D mapping
		template<> struct ResourceMapping < Texture2D > {

			/// \brief Concrete type associated to a Texture2D.
			using TMapped = DX11Texture2D;

		};

		/// \brief Render target mapping.
		template<> struct ResourceMapping < RenderTarget > {

			/// \brief Concrete type associated to a Render Target
			using TMapped = DX11RenderTarget;

		};

		/// \brief Mesh mapping.
		template<> struct ResourceMapping < Mesh > {

			/// \brief Concrete type associated to a Mesh.
			using TMapped = DX11Mesh;

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
		typename ObjectPtr<typename ResourceMapping<TResource>::TMapped> resource_cast(const ObjectPtr<TResource>& resource){

			return resource;

		}

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource>
		typename ObjectPtr<typename const ResourceMapping<TResource>::TMapped> resource_cast(const ObjectPtr<const TResource>& resource){

			return resource;

		}
		
		/////////////////////////////// DX11 RESOURCE VIEW TEMPLATE ///////////////////////////////

		template <typename TResource>
		inline DX11ResourceViewTemplate<TResource>::DX11ResourceViewTemplate(ObjectPtr<TResource> resource, ID3D11ShaderResourceView& resource_view) :
		resource_(resource),
		resource_view_(resource_view){}

		template <typename TResource>
		inline DX11ResourceViewTemplate<TResource>::~DX11ResourceViewTemplate(){}

		template <typename TResource>
		inline ID3D11ShaderResourceView& DX11ResourceViewTemplate<TResource>::GetShaderView(){

			return resource_view_;

		}

		/////////////////////////////// DX11 TEXTURE2D ///////////////////////////////

		inline unsigned int DX11Texture2D::GetWidth() const{

			return width_;

		}

		inline unsigned int DX11Texture2D::GetHeight()const {

			return height_;

		}

		inline unsigned int DX11Texture2D::GetMipMapCount() const{

			return mip_levels_;

		}

		inline ObjectPtr<IResourceView> DX11Texture2D::GetView(){

			return new DX11ResourceViewTemplate<DX11Texture2D>(this,
															   *shader_view_);

		}

		/////////////////////////////// DX11 RENDER TARGET ///////////////////////////////
		
		inline size_t DX11RenderTarget::GetSize() const{

			return std::accumulate(textures_.begin(), 
								   textures_.end(), 
								   static_cast<size_t>(0), 
								   [](size_t size, const ObjectPtr<DX11Texture2D>& texture){ 
				
										return size + texture->GetSize(); 
			
								   });
			
		}

		inline float DX11RenderTarget::GetAspectRatio() const{

			// The aspect ratio is guaranteed to be the same for all the targets.
			return static_cast<float>(textures_[0]->GetWidth()) /
				   static_cast<float>(textures_[0]->GetHeight());

		}

		inline AntialiasingMode DX11RenderTarget::GetAntialiasing() const{

			return antialiasing_;

		}

		inline unsigned int DX11RenderTarget::GetCount() const{

			return static_cast<unsigned int>(textures_.size());

		}

		inline ObjectPtr<Texture2D> DX11RenderTarget::GetTexture(int index){

			return textures_[index];

		}

		inline ObjectPtr<const Texture2D> DX11RenderTarget::GetTexture(int index) const{

			return textures_[index];

		}

		inline ObjectPtr<Texture2D> DX11RenderTarget::GetZStencil(){

			return zstencil_;

		}

		inline ObjectPtr<const Texture2D> DX11RenderTarget::GetZStencil() const{

			return zstencil_;

		}

	}

}