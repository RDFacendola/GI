/// \file resources.h
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
	
	/// \brief Describe the priority of the resources.
	enum class ResourcePriority{

		MINIMUM,		///< Lowest priority. These resources will be the first one to be freed when the system will run out of memory.
		LOW,			///< Low priority.
		NORMAL,			///< Normal priority. Default value.
		HIGH,			///< High priority.
		CRITICAL		///< Highest priority. These resources will be kept in memory at any cost.

	};

	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class Resources{

	public:

		/// \brief Folder where the resources are stored.
		static const wstring kResourceFolder;

		Resources();

		/// \brief Load a resource.

		/// Once loaded, a resource is immutable.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \param extra Extra loading parameters.
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

		using ResourceKey = std::pair < std::type_index, wstring >;
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

	/// \brief Base interface for graphical resources.
	/// \author Raffaele D. Facendola.
	class Resource{

	public:

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

		/// \brief Get the priority of the resource.
		/// \return Returns the resource priority.
		virtual ResourcePriority GetPriority() const = 0;

		/// \brief Set the priority of the resource.
		/// \param priority The new priority.
		virtual void SetPriority(ResourcePriority priority) = 0;

	};

	template <typename TResource>
	shared_ptr<TResource> Resources::LoadExtra(const wstring & path, const void * extras){

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