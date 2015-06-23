/// \file resources.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>

#include "object.h"

namespace gi_lib{

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

	/// \brief Base interface for graphical resources.
	/// Resources are reference counted.
	/// \author Raffaele D. Facendola.
	class IResource : public Object{

	public:

		virtual ~IResource(){}

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IResource(){}

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

	/// \brief Base interface for computation that can be executed on the GPU.
	/// \author Raffaele D. Facendola
	class GPUComputation : public IResource{

		/// \brief Structure used to compile a compute shader from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the compute shader code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Virtual destructor.
		virtual ~GPUComputation(){}

		/// \brief Execute the GPU program dispatching a grid of threads.
		/// The execution of the threads is guaranteed to be synchronous.
		/// \param x Threads to dispatch along the X-axis.
		/// \param y Threads to dispatch along the Y-axis.
		/// \param z Threads to dispatch along the Z-axis.
		/// \remarks The total amount of dispatched threads is x*y*z.
		/// \remarks The method will take care of dispatching the correct amount of thread group, based on the specified number of threads.
		virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) = 0;

		virtual ObjectPtr<IVariable> GetVariable(const string& name) = 0;

		virtual ObjectPtr<IVariable> GetResource(const string& name) = 0;		// Input, Read

		virtual ObjectPtr<IVariable> GetRWResource(const string& name) = 0;		// Input/Output, Read/Write

	};

	/// \brief Represents a strongly typed hardware vector.
	/// The vector can be written by the CPU and read by the GPU.
	/// The size of the vector does not change.
	/// \author Raffaele D. Facendola
	class StructuredVector : public IResource{

	public:

		/// \brief Structure used to build an empty structured vector from a description.
		struct FromDescription{

			NO_CACHE;

			size_t element_count;			///< \brief Elements inside the vector.

			size_t element_size;			///< \brief Size of each element.

		};

		/// \brief Virtual destructor.
		virtual ~StructuredVector(){}

		/// \brief Get the element count of this vector.
		/// \return Returns the element count of this vector.
		virtual size_t GetElementCount() const = 0;

		/// \brief Get the size of each element in bytes.
		/// \return Returns the size of each element.
		virtual size_t GetElementSize() const = 0;

		/// \brief Get the view to this resource.
		/// Use this view to bind the vector to the graphic pipeline.
		/// \return Returns a pointer to the resource view.
		virtual ObjectPtr<IResourceView> GetView() = 0;

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// \tparam TElement Type of the elements to map. Make sure to match the type this buffer was created with!.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		template <typename TElement>
		TElement* Lock();

		/// \brief Commit a mapped buffer back to the video memory, revoking CPU access.
		/// \remarks Unlocking the buffer will invalidate the system-memory pointer returned by the Lock method.
		virtual void Unlock() = 0;

	private:

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// The elements already present inside the vector are discarded.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		virtual void* LockDiscard() = 0;

	};

	///////////////////////////////////////// MATERIAL /////////////////////////////////////////

	template <typename TValue>
	inline void IVariable::Set(const TValue& value){

		Set(static_cast<const void*>(&value),
			sizeof(TValue));

	}
	
	///////////////////////////////////////// STRUCTURED VECTOR /////////////////////////////////////////

	template <typename TElement>
	inline TElement* StructuredVector::Lock(){

		return static_cast<TElement*>(LockDiscard());

	}
	
}