#include "graphics.h"

#include <numeric>
#include <map>

#include "exceptions.h"
#include "core.h"
#include "resources.h"
#include "range.h"

#include "dx11/dx11graphics.h"

using namespace std;
using namespace gi_lib;

namespace{

	/// \brief Unique key associated to a cached resource.
	struct ResourceCacheKey{

		/// \brief Type index associated to the resource.
		type_index resource_id_;

		/// \brief Type index associated to the resource's load arguments.
		type_index args_id_;

		/// \brief Unique cache key associated the current value of the load arguments.
		size_t cache_key_;

		/// \brief Default constructor.
		ResourceCacheKey() :
			resource_id_(type_index(typeid(void))),
			args_id_(type_index(typeid(void))),
			cache_key_(0){}

		/// \brief Explicit constructor.
		ResourceCacheKey(const type_index& resource_id, const type_index& args_id, size_t cache_key) :
			resource_id_(resource_id),
			args_id_(args_id),
			cache_key_(cache_key){}

		/// \brief Lesser comparison operator needed by the cache.
		bool operator<(const ResourceCacheKey& other) const{

			return memcmp(this,
						  &other,
						  sizeof(ResourceCacheKey)) < 0;

		}

	};

	/// \brief The resource cached simply maps a cache key to the proper resource.
	/// \brief A cached resource is a weak smart pointer. If there's at least one pointer alive, the cached resource will be alive as well.
	/// The cached resource is invalidated when the last pointer is destroyed.
	using ResourceCache = map < const ResourceCacheKey, ObjectWeakPtr < IResource > >;

	/// \brief List of the loaded resources.
	using ResourceList = vector < ObjectWeakPtr < IResource > >;

}

///////////////////////// COLOR ////////////////////////////////////////

const Color kOpaqueWhite = Color(1.0f, 1.0f, 1.0f, 1.0f);

const Color kOpaqueBlack = Color(0.0f, 0.0f, 0.0f, 1.0f);

const Color kTransparentBlack = Color(0.0f, 0.0f, 0.0f, 0.0f);

//////////////////////// RESOURCES :: IMPL //////////////////////////////

/// \brief Private implementation of the Resources class.
/// Cached resources are stored with two weak pointers (one for the cache and one for the resource list).
/// Resource requiring no cache are stored with only one weak pointer (for the resource list).
struct Resources::Impl{

	Impl(Resources& subject);

	Impl& operator=(const Impl&) = delete;

	/// \brief Fetch a resource from cache.
	/// \return Returns the cached resource if any, returns nullptr if the resource was expired or the key was not present.
	ObjectPtr<IResource> FetchFromCache(const ResourceCacheKey& cache_key){

		auto it = resource_cache_.find(cache_key);

		if (it != resource_cache_.end()){

			if (auto resource = it->second.Lock()){

				// The resource was found and is still valid.
				return resource;

			}

			// Expired resource, erase the line from the cache.
			resource_cache_.erase(it);

		}

		// The cache key is invalid.
		return nullptr;

	}

	/// \brief Get the loaded resources
	Range<ResourceList::const_iterator> GetResources() const{

		EvictInvalidResources();

		return Range<ResourceList::const_iterator>(resource_list_.cbegin(),
												   resource_list_.cend());

	}
	
	/// \brief Store a cached resource.
	void StoreResource(const ResourceCacheKey& cache_key, ObjectWeakPtr<IResource> resource){

		resource_cache_[cache_key] = resource;		// Cache.

		StoreResource(resource);					// Resource list.

	}

	/// \brief Store a resource.
	void StoreResource(ObjectWeakPtr<IResource> resource){

		resource_list_.push_back(resource);			// Resource list.

	}

private:

	/// \brief Removes the invalid resources from the resource list and cache.
	void EvictInvalidResources() const{

		resource_list_.erase(std::remove_if(resource_list_.begin(),
											resource_list_.end(),
											[](const ObjectWeakPtr<IResource>& resource){

												return resource.IsValid();
											
											}),
							 resource_list_.end());
			
	}

	mutable ResourceCache resource_cache_;		/// \brief List of cached resources.

	mutable ResourceList resource_list_;		/// \brief List of the loaded resources, either cached or not. Needed to compute the correct memory consumption.

	Resources& subject_;						///< \brief Subject of the implementation.

};

Resources::Impl::Impl(Resources& subject) :
subject_(subject){}

//////////////////////// RESOURCES //////////////////////////////

Resources::Resources() :
pimpl_(make_unique<Impl>(*this)){}

Resources::~Resources(){}

size_t Resources::GetSize() const{

	// Runs trough every resource and sum its memory footprint.

	auto resources = pimpl_->GetResources();

	return accumulate(resources.begin(),
					  resources.end(),
					  static_cast<size_t>(0),
					  [](size_t accumulator, const ObjectWeakPtr<IResource>& resource){

							if (auto locked_resource = resource.Lock()){

								accumulator += locked_resource->GetSize();

							}

							return accumulator;

					  });

}

ObjectPtr<IResource> Resources::LoadFromCache(const type_index& resource_type, const type_index& args_type, const void* args, size_t cache_key){

	ResourceCacheKey key = { resource_type, 
							 args_type, 
							 cache_key };

	auto resource = pimpl_->FetchFromCache(key);

	if (!resource){

		// The resource is not cached. Load it.

		resource = Load(resource_type,
						args_type,
						args);

		// If the resource type is not supported, the result of Load is nullptr.

		if (resource){

			pimpl_->StoreResource(key,
								  resource);

		}

	}
	
	return resource;
	
}

ObjectPtr<IResource> Resources::LoadDirect(const type_index& resource_type, const type_index& args_type, const void* args){

	auto resource = Load(resource_type,
						 args_type,
						 args);

	// If the resource type is not supported, the result of Load is nullptr.

	if (resource){

		pimpl_->StoreResource(resource);

	}
	
	return resource;

}

////////////////////// GRAPHICS ////////////////////////////////

Graphics::Graphics(){}

Graphics::~Graphics(){}

Graphics& Graphics::GetAPI(API api){

	switch (api){

		case API::DIRECTX_11:

			return dx11::DX11Graphics::GetInstance();

		default:

			THROW(L"Specified API is not supported.");

	}

}