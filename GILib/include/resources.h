/// \file resources.h
/// \brief Declare classes and interfaces used to manage graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <map>
#include <memory>
#include <type_traits>
#include <typeindex>

using ::std::wstring;
using ::std::map;
using ::std::weak_ptr;
using ::std::shared_ptr;

namespace gi_lib{

	class Resource;
	class Texture2D;
	class Texture3D;
	class Mesh;
	
	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class Resources{

	public:

		/// \brief Load a resource.

		/// Once loaded, a resource is immutable.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource>
		shared_ptr<TResource> Load(const wstring & path, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr);

	protected:

		/// \brief Load a resource.
		
		/// \param path The path of the resource.
		/// \param type_index Type of the resource to create.
		virtual shared_ptr<Resource> Load(const wstring & path, const std::type_index & type_index) = 0;

		/// \brief Type of the key associated to resources' path.
		using PathKey = wstring;

		// Map of the immutable resources
		map < PathKey, weak_ptr < Resource > > resources_;

	};

	/// \brief Base interface for graphical resources.
	/// \author Raffaele D. Facendola.
	class Resource{

	public:

	};

	/// \brief Plain texture interface.
	/// \author Raffaele D. Facendola.
	class Texture2D: public Resource{

	public:

		int value;

	};

	/// \brief Volumetric texture interface.
	/// \author Raffaele D. Facendola.
	class Texture3D{

	public:

	};

	/// \brief 3D model interface.
	/// \author Raffaele D. Facendola.
	class Mesh{

	public:

	};

	//

	template <typename TResource>
	shared_ptr<TResource> Resources::Load(const wstring & path, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type*){

		//Check if the resource exists inside the map
		auto it = resources_.find(path);

		if (it != resources_.end()){

			if (auto resource = it->second.lock()){

				return static_pointer_cast<TResource>(resource);

			}

			//Resource was expired...

		}

		auto resource = Load(path, std::type_index(typeid(TResource)));

		resources_[path] = resource;	// Conversion to weak_ptr

		return resource;

	}

}