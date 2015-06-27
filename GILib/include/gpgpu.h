/// \file gpgpu.h
/// \brief This file contains the interfaces for general-purpose computing on GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeindex>
#include <type_traits>

#include "object.h"
#include "resources.h"
#include "enums.h"

#include "gilib.h"
#include "fnv1.h"

namespace gi_lib{

	ENUM_FLAGS(GPUAccess, char){

		Read = (1 << 0),				///< \brief Grants read-only permission.
		Write = (1 << 1),				///< \brief Grants write permissions.

		Random = Read | Write,			///< \brief Grants read-and-write permissions.

	};

	template <typename TResource, GPUAccess access, typename>
	class IComputationArgument;

	/// \brief Base interface for GPU computations.
	/// \author Raffaele D. Facendola
	class IComputation : public IResource{

	public:

		/// \brief Structure used to compile a compute shader from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the compute shader code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Virtual destructor.
		virtual ~IComputation(){};
		
		/// \brief Get a computation argument by type and access permissions.
		/// \tparam TArgument Type of the argument to get.
		/// \tparam access Type of access required by the GPU.
		/// \param name Name of the argument to get.
		/// \return Returns a pointer to the argument setter if such argument exists, returns nullptr otherwise.
		/// \remarks The method fails if the specified type and/or access permissions are wrong, even if the argument exists.
		template <typename TArgument, GPUAccess access = GPUAccess::Read>
		ObjectPtr<IComputationArgument<TArgument, access, void>> GetArgument(const string& name);
				
		/// \brief Execute the computation on the GPU.
		/// \param x Threads to dispatch along the X-axis.
		/// \param y Threads to dispatch along the Y-axis.
		/// \param z Threads to dispatch along the Z-axis.
		/// \remarks The total amount of dispatched threads is x*y*z.
		virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) = 0;

	private:

		/// \brief Get a pointer to a computation resource.
		/// \param name Name of the resource to get.
		/// \param resource_type Type of the resource to get.
		/// \param access Type of access required by the computation.
		/// \remarks The returned object's type must be compatible with IComputationResource<resource_type, access>.
		virtual ObjectPtr<Object> GetArgument(const string& name, const std::type_index& argument_type, GPUAccess access) = 0;

	};

	template <typename TArgument, GPUAccess access, class enable>
	class IComputationArgument;

	template <typename TArgument, GPUAccess access>
	class IComputationArgument < TArgument, access, typename std::enable_if<std::is_base_of<Object, TArgument>::value>::type > : public Object{

	public:

		using ResourceView = IResourceView;

		/// \brief Virtual destructor.
		virtual ~IComputationArgument(){}

		virtual void Set(ObjectPtr<ResourceView> resource_view) = 0;

	};

	template <typename TArgument, GPUAccess access>
	class IComputationArgument < TArgument, access, typename std::enable_if<std::is_arithmetic<TArgument>::value>::type > : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IComputationArgument(){}

		virtual void Set(TArgument value) = 0;

	};

	template <typename TArgument, GPUAccess access>
	class IComputationArgument < TArgument, access, typename std::enable_if<!std::is_arithmetic<TArgument>::value &&
																			!std::is_base_of<Object, TArgument>::value>::type > : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IComputationArgument(){}

		virtual void Set(const TArgument& value) = 0;

	};
	
	/////////////////////////////////// ICOMPUTATION ///////////////////////////////////

	template <typename TArgument, GPUAccess access>
	inline ObjectPtr<IComputationArgument<TArgument, access, void>> IComputation::GetArgument(const string& name){

		return GetArgument(name,
						   type_index(typeid(TArgument)),
						   access);

	}

	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IComputation::CompileFromFile::GetCacheKey() const{

		return ::hash::fnv_1{}(to_string(file_name));

	}

}