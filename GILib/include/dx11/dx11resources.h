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
		



		/// \brief DirectX11 resource mapping template.
		template<typename TResource> struct ResourceMapping;

		/// \brief Shader resource mapping.
		template<> struct ResourceMapping < IResourceView > {

			/// \brief Concrete type associated to a shader resource.
			using TMapped = DX11ResourceView;

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
				


	}

}