/// \file dx11resource_traits.h
/// \brief Templates to manage DirectX11 resources type traits. 
///
/// \author Raffaele D. Facendola

#include <memory>

using ::std::shared_ptr;
using ::std::unique_ptr;

namespace gi_lib{

	class Resource;

	class Texture2D;

	namespace dx11{

		class DX11Texture2D;

		/// \brief DirectX11 resource traits
		template<typename TResource> struct resource_traits;

		/// \brief DirectX11 resource traits
		template<> struct resource_traits < Texture2D > {

			using type = DX11Texture2D;

		};

		/// \brief Performs a resource cast from an abstract type to a concrete type.
		/// \tparam TResource Type of the resource to cast.
		/// \param resource The shared pointer to the resource to cast.
		/// \return Returns a shared pointer to the casted resource.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		typename shared_ptr<typename resource_traits<TResource>::type> resource_cast(shared_ptr<TResource> & resource){

			return static_pointer_cast<typename resource_traits<TResource>::type> (resource.operator->());

		}

	}

}