/// \file resources.h
/// \brief Declare classes and interfaces used to manage graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <stddef.h>
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

		/// \brief Get the amount of memory used by the resources loaded.
		size_t GetSize();

	protected:

		/// \brief Load a resource.
		
		/// \param path The path of the resource.
		/// \param type_index Type of the resource to create.
		virtual shared_ptr<Resource> Load(const wstring & path, const std::type_index & type_index) = 0;

		/// \brief Type of the key associated to resources' path.
		using PathKey = wstring;

		/// \brief Type of the map used to store the cached resources.
		using ResourceMap = map < PathKey, weak_ptr < Resource > > ;

		// Map of the immutable resources
		ResourceMap resources_;

	};

	/// \brief Describe the priority of the resources.
	enum class ResourcePriority{

		MINIMUM,		///< Lowest priority. These resources will be the first one to be freed when the system will run out of memory.
		LOW,			///< Low priority.
		NORMAL,			///< Normal priority. Default value.
		HIGH,			///< High priority.
		CRITICAL		///< Highest priority. These resources will be kept in memory at any cost.
		
	};

	/// \brief Base interface for graphical resources.
	/// \author Raffaele D. Facendola.
	class Resource{

	public:

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() = 0;

		/// \brief Get the priority of the resource.
		/// \return Returns the resource priority.
		virtual ResourcePriority GetPriority() = 0;

		/// \brief Set the priority of the resource.
		/// \param priority The new priority.
		virtual void SetPriority(ResourcePriority priority) = 0;

	};

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