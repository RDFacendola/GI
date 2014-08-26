/// \file resource_manager.h
/// \brief Declare classes and interfaces used to manage graphical resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <stddef.h>
#include <string>
#include <map>
#include <memory>
#include <typeindex>

using ::std::wstring;
using ::std::map;
using ::std::unique_ptr;
using ::std::weak_ptr;
using ::std::shared_ptr;

namespace gi_lib{

	class Resource;

	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class ResourceManager{

	public:

		/// \brief Folder where the resources are stored.
		static const wstring kResourceFolder;

		ResourceManager();

		/// \brief Load a resource.

		/// Once loaded, a resource is immutable.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \param extras Extra loading parameters.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		shared_ptr<TResource> Load(const wstring & path, const typename TResource::Extra & extras){

			return LoadExtra<TResource>(path, &extras);

		}
		
		/// \brief Load a resource.

		/// Once loaded, a resource is immutable.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		shared_ptr<TResource> Load(const wstring & path){

			return LoadExtra<TResource>(path, nullptr);

		}

		/// \brief Get the amount of memory used by the resources loaded.
		size_t GetSize();

	protected:

		/// \brief Type of resources' keys.
		using ResourceKey = std::pair < std::type_index, wstring >;

		/// \brief Type of resources' map.
		using ResourceMap = map < ResourceKey, weak_ptr<Resource> >;

		/// \brief Load a resource.
		
		/// \param key Unique key of the resource to load.
		/// \param extras Extra parameters.
		/// \return Returns a pointer to the loaded resource
		virtual unique_ptr<Resource> LoadDirect(const ResourceKey & key, const void * extras) = 0;

	private:

		template <typename TResource>
		shared_ptr<TResource> LoadExtra(const wstring & path, const void * extras);

		// Map of the immutable resources
		ResourceMap resources_;

		// Base path for the resources
		wstring base_path_;

	};

	//

	template <typename TResource>
	shared_ptr<TResource> ResourceManager::LoadExtra(const wstring & path, const void * extras){

		//Check if the resource exists inside the map
		auto key = make_pair(std::type_index(typeid(TResource)), base_path_ + path);

		auto it = resources_.find(key);

		if (it != resources_.end()){

			if (auto resource = it->second.lock()){

				return static_pointer_cast<TResource>(resource);

			}

			//Resource was expired...

		}

		auto resource = shared_ptr<Resource>(std::move(LoadDirect(key, extras)));		// To shared ptr

		resources_[key] = resource;														// To weak ptr

		return static_pointer_cast<TResource>(resource);								// To shared ptr (of the requested type). Evil downcasting :D

	}

}