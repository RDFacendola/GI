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
#include "material.h"
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
		
		class DX11MaterialVariable;
		class DX11MaterialResource;



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
		class DX11StructuredVector : public IDynamicBuffer{

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

		/// \brief Structured vector mapping.
		template<> struct ResourceMapping < IDynamicBuffer > {

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