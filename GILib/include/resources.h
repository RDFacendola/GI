/// \file resources.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>

#include "object.h"

namespace gi_lib{

	/// \brief Base interface for graphical resources.
	/// Resources are reference counted.
	/// You may improve this class to provide shared functionalities.
	/// \author Raffaele D. Facendola.
	class IResource : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IResource() = 0 {};

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;
		
	};

	/// \brief Base interface for resource views.
	/// A resource view is used to bind resources to the graphic pipeline, granting read-only access to the GPU.
	/// \author Raffaele D. Facendola.
	class IResourceView : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IResourceView() = 0 {};
		
	};






	/// \brief Base interface for resource read/write views.
	/// This kind of resource view is used to bind the resource to the graphic pipeline, granting both read and write access.
	/// \author Raffaele D. Facendola.
	class IResourceRWView{

	public:

		/// \brief Virtual destructor.
		virtual ~IResourceRWView() = 0 {};
		
	};
	


	using ::std::wstring;
	using ::std::string;
	using ::std::vector;

	/// \brief Macro used to declare that the bundle will use the caching mechanism.
	#define USE_CACHE \
	using use_cache = void

	/// \brief Macro used to declare that the bundle won't use the caching mechanism.
	#define NO_CACHE \
	using no_cache = void

	/// \brief If T declares a type "use_cache", use_cache has a public member "type", otherwise there's no member.
	template <typename T, typename T::use_cache* = nullptr>
	struct use_cache{

		using type = void;

	};

	/// \brief If T declares a type "no_cache", no_cache has a public member "type", otherwise there's no member.
	template <typename T, typename T::no_cache* = nullptr>
	struct no_cache{

		using type = void;

	};

	
}