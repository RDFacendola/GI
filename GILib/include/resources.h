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
		virtual ~IResource(){}

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IResource(){}

	};

	/// GPU Read		- SRV - IReadAccessView
	/// GPU Read/Write	- UAV - IRandomAccessView
	/// CPU Read/Write	- Map -	

	/// \brief Resource view used to bind resources to the graphic pipeline as read-only resources.
	/// Resource views should keep a reference to their relative resource.
	/// \author Raffaele D. Facendola
	class IResourceView : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IResourceView(){}

	protected:

		/// \brief protected constructor, prevent instantiation.
		IResourceView(){}

	};

	/// \brief Resource view used to bind resources to the graphic pipeline as read/write resources.
	/// Resource views should keep a reference to their relative resource.
	/// \author Raffaele D. Facendola
	class IResourceRandomAccessView : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IResourceRandomAccessView(){}

	protected:

		/// \brief protected constructor, prevent instantiation.
		IResourceRandomAccessView(){}

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
	
	/// \brief Interface for variables.
	/// \author Raffaele D. Facendola.
	class IVariable : public Object{

	public:

		/// \brief Default destructor.
		virtual ~IVariable(){}

		/// \brief Set the variable value.
		/// \param value The value to set.
		template <typename TValue>
		void Set(const TValue& value);

		/// \brief Set the variable value.
		/// \param buffer Pointer to the buffer holding the data to write.
		/// \param size Size of the buffer.
		virtual void Set(const void* buffer, size_t size) = 0;

	};

	/// \brief Interface for resources.
	/// \author Raffaele D. Facendola.
	class IResourceBLAH : public Object{

	public:

		/// \brief Default destructor.
		virtual ~IResourceBLAH(){}

		/// \brief Set the resource value.
		/// \param resource The resource to bind to the material.
		virtual void Set(ObjectPtr<IResourceView> resource) = 0;

	};

	class IUnorderedAccess : public Object{

	public:

		virtual ~IUnorderedAccess(){}

		virtual void Set(ObjectPtr<IResourceRandomAccessView> unordered) = 0;

	};

	///////////////////////////////////////// MATERIAL /////////////////////////////////////////

	template <typename TValue>
	inline void IVariable::Set(const TValue& value){

		Set(static_cast<const void*>(&value),
			sizeof(TValue));

	}
	
}