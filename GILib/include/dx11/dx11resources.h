/// \file dx11resources.h
/// \brief Classes and methods for DirectX11 resource management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>
#include <string>
#include <map>
#include <memory>
#include <numeric>

#include "dx11.h"

#include "graphics.h"
#include "resources.h"

#include "mesh.h"
#include "texture.h"
#include "render_target.h"
#include "buffer.h"

#include "gilib.h"

#include "windows/win_os.h"

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
			virtual ID3D11ShaderResourceView* GetShaderView() = 0;

			/// \brief Get the unordered access view associated to this resource.
			/// \return Returns the unordered access view associated to this resource.
			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() = 0;

		};

		/// \brief Concrete templated resource view class.
		/// This class will hold a strong reference to a resource (so it is is not released while the view is still being used somewhere)
		/// and its shader resource view.
		template <typename TResource>
		class DX11ResourceViewTemplate : public DX11ResourceView{

		public:

			/// \brief Create a new resource view from a concrete resource type.
			/// \param resource Resource associated to the view.
			/// \param resource_view Shader resource view relative to the specified resource.
			/// \param unordered_access Unordered access view relative to the specified resource.
			DX11ResourceViewTemplate(ObjectPtr<TResource> resource, ID3D11ShaderResourceView* resource_view, ID3D11UnorderedAccessView* unordered_access);
			
			/// \brief Virtual destructor.
			virtual ~DX11ResourceViewTemplate();

			virtual ID3D11ShaderResourceView* GetShaderView() override;

			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() override;

		private:

			ObjectPtr<TResource> resource_;					///< \brief Strong reference to the resource.

			ID3D11ShaderResourceView* resource_view_;		///< \brief Shader resource view.

			ID3D11UnorderedAccessView* unordered_access_;	///< \brief Unordered access view.
			
		};
		
		/// \brief DirectX11 plain texture.
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

			/// \brief Create a new texture with unordered access.
			/// \param texture The texture to bind.
			/// \param shader_view The view used to bind the texture to the shader.
			/// \param unordered_view The view used to bind the texture as unordered access.
			DX11Texture2D(ID3D11Texture2D& texture, ID3D11ShaderResourceView& shader_view, ID3D11UnorderedAccessView& unordered_view);
			
			/// \brief Create a new texture from an existing DirectX11 texture.
			/// \param texture The DirectX11 texture.
			/// \param format The format used when sampling from the texture.
			DX11Texture2D(ID3D11Texture2D& texture, DXGI_FORMAT format);

			virtual ~DX11Texture2D(){}

			virtual size_t GetSize() const override;

			virtual unsigned int GetWidth() const override;

			virtual unsigned int GetHeight() const override;

			virtual unsigned int GetMipMapCount() const override;

			virtual ObjectPtr<IResourceView> GetView() const override;

			DXGI_FORMAT GetFormat() const;

		private:

			void UpdateDescription();

			unique_ptr<ID3D11Texture2D, COMDeleter> texture_;						///< \brief Pointer to the actual texture.

			unique_ptr<ID3D11ShaderResourceView, COMDeleter> shader_view_;			///< \brief Pointer to the shader resource view of the texture.

			unique_ptr<ID3D11UnorderedAccessView, COMDeleter> unordered_access_;	///< \brief Pointer to the unordered access view of the texture. May be null.

			unsigned int width_;													///< \brief Width of the texture, in pixels.

			unsigned int height_;													///< \brief Height of the texture, in pixels.

			unsigned int bits_per_pixel_;											///< \brief Bits per pixel.
				
			unsigned int mip_levels_;												///< \brief Mip levels.

			DXGI_FORMAT format_;													///< \brief Surface format.

		};

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

			virtual AntialiasingMode GetAntialiasing() const override;

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

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh : public IStaticMesh{

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

			virtual ObjectPtr<IVariable> GetVariable(const string& name) override;

			virtual ObjectPtr<IResourceBLAH> GetResource(const string& name) override;

			/// \brief Commit all the constant buffers and bind the material to the pipeline.
			void Commit(ID3D11DeviceContext& context);

		private:

			/// \brief Holds the properties shared among material instances.
			struct MaterialImpl;

			/// \brief Holds the private properties of this material instance.
			struct InstanceImpl;

			/// \brief Material variable.
			class DX11MaterialVariable : public IVariable{

			public:

				DX11MaterialVariable(InstanceImpl& instance_impl, size_t buffer_index, size_t variable_size, size_t variable_offset);

				virtual void Set(const void * buffer, size_t size) override;

			private:

				InstanceImpl* instance_impl_;

				size_t buffer_index_;

				size_t variable_offset_;		///< \brief Offset of the variable from the beginning of the buffer in bytes.

				size_t variable_size_;			///< \brief Size of the variable in bytes.

			};

			/// \brief Material resource, used to bind shader resource views.
			class DX11MaterialResource : public IResourceBLAH{

			public:

				DX11MaterialResource(InstanceImpl& instance_impl, size_t resource_index);

				virtual void Set(ObjectPtr<IResourceView> resource) override;

			private:

				InstanceImpl* instance_impl_;

				size_t resource_index_;

			};

			/// \brief Material resource, used to bind unordered access views.
			class DX11MaterialUAV : public IResourceBLAH{

			public:

				DX11MaterialUAV(InstanceImpl& instance_impl, size_t uav_index);

				virtual void Set(ObjectPtr<IResourceView> resource) override;

			private:

				InstanceImpl* instance_impl_;

				size_t uav_index_;

			};
			
			shared_ptr<MaterialImpl> shared_impl_;		///< \brief Properties shared among material instances.

			unique_ptr<InstanceImpl> private_impl_;		///< \brief Private properties of this material instance.

		};

		/// \brief Represents a DirectX11 sampler state.
		/// \author Raffaele D. Facendola.
		class DX11Sampler : public IResource{

		public:

			/// \brief Structure used to create a sampler state from a plain description.
			struct FromDescription{

				USE_CACHE;

				/// \brief Texture mapping.
				TextureMapping texture_mapping;

				/// \brief Anisotropy level.
				unsigned int anisotropy_level;

				/// \brief Get the cache key associated to the structure.
				/// \return Returns the cache key associated to the structure.
				size_t GetCacheKey() const;

			};

			/// \brief Create a sampler state from a plain description.
			DX11Sampler(const FromDescription& description);

			virtual size_t GetSize() const override;

			/// \brief Get the sampler state.
			/// \return Returns the sampler state.
			ID3D11SamplerState& GetSamplerState() const;

		private:

			unique_ptr<ID3D11SamplerState, COMDeleter> sampler_state_;

		};

		/// \brief DirectX11 structured vector.
		/// \author Raffaele D. Facendola
		class DX11StructuredVector : public StructuredBuffer{

		public:

			/// \brief Create a new DirectX11 material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11StructuredVector(const FromDescription& args);

			/// \brief Default destructor.
			~DX11StructuredVector();

			virtual size_t GetSize() const override;

			virtual size_t GetElementCount() const override;

			virtual size_t GetElementSize() const override;

			virtual ObjectPtr<IResourceView> GetView() override;

			virtual void Unlock() override;

			/// \brief Map the vector to the system memory.
			/// \param context Context used to map the buffer.
			/// \return Returns a pointer to the first element of the vector.
			template <typename TElement>
			TElement* Map(ID3D11DeviceContext& context);

			/// \brief Unmap the vector from the system memory and commits it back to the video memory.
			/// \param context Context used to unmap the buffer.
			void Unmap(ID3D11DeviceContext& context);
			
			/// \brief Write the uncommitted state to the structured buffer.
			/// \param context Context used to commit the buffer.
			void Commit(ID3D11DeviceContext& context);

		private:

			virtual void* LockDiscard() override;

			size_t element_count_;												///< \brief Number of elements inside the vector.

			size_t element_size_;												///< \brief Size of each element.

			void* data_;														///< \brief Pointer to the raw buffer in system memory.

			mutable bool dirty_;												///< \brief Whether the buffer contains dirty data that needs to be committed.

			unique_ptr<ID3D11Buffer, COMDeleter> buffer_;						///< \brief Pointer to the structured buffer.

			unique_ptr<ID3D11ShaderResourceView, COMDeleter> shader_view_;		///< \brief Pointer to the shader resource view of the vector.

		};

		/// \brief DirectX11 resource mapping template.
		template<typename TResource> struct ResourceMapping;

		/// \brief Shader resource mapping.
		template<> struct ResourceMapping < IResourceView > {

			/// \brief Concrete type associated to a shader resource.
			using TMapped = DX11ResourceView;

		};

		/// \brief Texture 2D mapping
		template<> struct ResourceMapping < ITexture2D > {

			/// \brief Concrete type associated to a Texture2D.
			using TMapped = DX11Texture2D;

		};

		/// \brief Render target mapping.
		template<> struct ResourceMapping < IRenderTarget > {

			/// \brief Concrete type associated to a Render Target
			using TMapped = DX11RenderTarget;

		};

		/// \brief Mesh mapping.
		template<> struct ResourceMapping < IStaticMesh > {

			/// \brief Concrete type associated to a Mesh.
			using TMapped = DX11Mesh;

		};

		/// \brief Material mapping.
		template<> struct ResourceMapping < Material > {

			/// \brief Concrete type associated to a Material.
			using TMapped = DX11Material;

		};

		/// \brief Structured vector mapping.
		template<> struct ResourceMapping < StructuredBuffer > {

			/// \brief Concrete type associated to a structured vector.
			using TMapped = DX11StructuredVector;

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

		/// \brief Performs a resource cast and extract a shader resource view.
		/// The concrete resource should expose a method GetView() : DX11ResourceView&.
		/// \return Returns a pointer to the shader resource view associated to the specified resource.
		template <typename TResource>
		typename ID3D11ShaderResourceView* resource_srv(const ObjectPtr<TResource>& resource){

			return resource_cast(resource->GetView())->GetShaderView();

		}

		/// \brief Performs a resource cast and extract an unordered access view.
		/// The concrete resource should expose a method GetView() : DX11ResourceView&.
		/// \return Returns a pointer to the unordered access view associated to the specified resource.
		template <typename TResource>
		typename ID3D11UnorderedAccessView* resource_uav(const ObjectPtr<TResource>& resource){

			return resource_cast(resource->GetView())->GetUnorderedAccessView();

		}
		
		/////////////////////////////// DX11 RESOURCE VIEW TEMPLATE ///////////////////////////////

		template <typename TResource>
		inline DX11ResourceViewTemplate<TResource>::DX11ResourceViewTemplate(ObjectPtr<TResource> resource, ID3D11ShaderResourceView* resource_view, ID3D11UnorderedAccessView* unordered_access) :
			resource_(resource),
			resource_view_(resource_view),
			unordered_access_(unordered_access){}

		template <typename TResource>
		inline DX11ResourceViewTemplate<TResource>::~DX11ResourceViewTemplate(){}

		template <typename TResource>
		inline ID3D11ShaderResourceView* DX11ResourceViewTemplate<TResource>::GetShaderView(){

			return resource_view_;

		}

		template <typename TResource>
		inline ID3D11UnorderedAccessView* DX11ResourceViewTemplate<TResource>::GetUnorderedAccessView(){

			return unordered_access_;

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

		inline DXGI_FORMAT DX11Texture2D::GetFormat() const{

			return format_;

		}

		inline ObjectPtr<IResourceView> DX11Texture2D::GetView() const{

			return new DX11ResourceViewTemplate<const DX11Texture2D>(this,
																	 shader_view_.get(),
																	 unordered_access_.get());

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

		inline ObjectPtr<ITexture2D> DX11RenderTarget::operator[](size_t index){

			return textures_[index];

		}

		inline ObjectPtr<const ITexture2D> DX11RenderTarget::operator[](size_t index) const{

			return textures_[index];

		}

		inline ObjectPtr<ITexture2D> DX11RenderTarget::GetZStencil(){

			return zstencil_;

		}

		inline ObjectPtr<const ITexture2D> DX11RenderTarget::GetZStencil() const{

			return zstencil_;

		}

		inline unsigned int DX11RenderTarget::GetWidth() const{

			return textures_[0]->GetWidth();

		}

		inline unsigned int DX11RenderTarget::GetHeight() const{

			return textures_[0]->GetHeight();

		}
		
		/////////////////////////////// DX11 SAMPLER ///////////////////////////////

		inline ID3D11SamplerState& DX11Sampler::GetSamplerState() const{

			return *sampler_state_;

		}

		inline size_t DX11Sampler::GetSize() const
		{

			return 0;

		}

		////////////////////////////// DX11 STRUCTURED VECTOR //////////////////////////////////

		inline size_t DX11StructuredVector::GetSize() const{

			return element_count_ * element_size_;

		}

		inline size_t DX11StructuredVector::GetElementCount() const{

			return element_count_;

		}
		
		inline size_t DX11StructuredVector::GetElementSize() const{

			return element_size_;

		}

		inline ObjectPtr<IResourceView> DX11StructuredVector::GetView(){

			return new DX11ResourceViewTemplate<DX11StructuredVector>(this,
																	  shader_view_.get(),
																	  nullptr);

		}

		template <typename TElement>
		inline TElement* DX11StructuredVector::Map(ID3D11DeviceContext& context){

			dirty_ = true;

			D3D11_MAPPED_SUBRESOURCE subresource;

			context.Map(buffer_.get(),
						0,								// Map everything
						D3D11_MAP_WRITE_DISCARD,		// Discard the previous content.
						0,
						&subresource);

			return static_cast<TElement*>(subresource.pData);

		}
		
	}

}