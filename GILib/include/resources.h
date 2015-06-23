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
	/// You may improve this class to provide shared functionalities to every resource.
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

	/// \brief A resource view, used to bind resources to the graphic pipeline (read-only).
	/// Resource views are reference counted.
	/// \author Raffaele D. Facendola
	class IResourceView : public Object{

	public:

		/// \brief Needed for virtual classes.
		virtual ~IResourceView(){}

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IResourceView(){}

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

	/// \brief Base interface for materials.
	/// \author Raffaele D. Facendola
	class Material : public IResource{

	public:

		/// \brief Structure used to compile a material from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the material code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Structure used to instantiate an existing material.
		struct Instantiate{

			NO_CACHE;

			ObjectPtr<Material> base;	///< \brief Material to instantiate.

		};

		/// \brief Virtual destructor.
		virtual ~Material(){}

		/// \brief Get a material variable by name.
		/// \param name The name of the variable.
		/// \return Returns a pointer to the variable matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<IVariable> GetVariable(const string& name) = 0;

		/// \brief Get a material resource by name.
		/// \param name The name of the resource.
		/// \return Returns a pointer to the resource matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<IResourceBLAH> GetResource(const string& name) = 0;

	};

	///////////////////////////////////////// MATERIAL /////////////////////////////////////////

	template <typename TValue>
	inline void IVariable::Set(const TValue& value){

		Set(static_cast<const void*>(&value),
			sizeof(TValue));

	}
	
}