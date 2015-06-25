/// \file gpgpu.h
/// \brief This file contains the interfaces for general-purpose computing on GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"
#include "resources.h"

#include "gilib.h"
#include "fnv1.h"

namespace gi_lib{

	class IComputationParameter;
	class IComputationResource;
	class IComputationOutput;

	/// \brief Base interface for GPU computations.
	/// \author Raffaele D. Facendola
	class IComputation : public IResource{

		/// \brief Structure used to compile a compute shader from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the compute shader code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Virtual destructor.
		virtual ~IComputation() = 0;

		/// \brief Get a pointer to a computation parameter.
		/// \param name Name of the parameter to get.
		/// \return Returns a pointer to the computation parameter if any, returns nullptr otherwise.
		virtual ObjectPtr<IComputationParameter> GetParameter(const string& name) = 0;

		/// \brief Get a pointer to a computation resource.
		/// \param name Name of the resource to get.
		/// \return Returns a pointer to the computation resource if any, returns nullptr otherwise.
		/// \remarks The resource is accessed in read-only mode.
		virtual ObjectPtr<IComputationResource> GetResource(const string& name) = 0;

		/// \brief Get a pointer to a computation output.
		/// \param name Name of the computation output to get.
		/// \return Returns a pointer to the computation output if any, returns nullptr otherwise.
		virtual ObjectPtr<IComputationOutput> GetOutput(const string& name) = 0;

		/// \brief Execute the computation on the GPU.
		/// \param x Threads to dispatch along the X-axis.
		/// \param y Threads to dispatch along the Y-axis.
		/// \param z Threads to dispatch along the Z-axis.
		/// \remarks The total amount of dispatched threads is x*y*z.
		virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) = 0;

	};

	/// \brief Base interface for computation parameters.
	/// The class is used to change the value of a computation parameter.
	/// \author Raffaele D. Facendola
	class IComputationParameter : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IComputationParameter() = 0;

		/// \brief Set a new value for the computation parameter.
		/// \tparam TParameter Type of the parameter to write.
		/// \param value Value to write.
		template <typename TParameter>
		void Set(const TParameter& value);

	private:

		/// \brief Set a new value for the computation parameter.
		/// \param value_ptr Pointer to the value to write.
		/// \param size Size of the buffer containing the value to write.
		virtual void Set(const void* value_ptr, size_t size) = 0;

	};

	/// \brief Base interface for computation resources.
	/// The class is used to bind a computation resource.
	/// \author Raffaele D. Facendola
	class IComputationResource : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IComputationResource() = 0;

		/// \brief Bind a new resource to the computation.
		/// \param resource Read-only view of the resource to bind to the computation.
		virtual void Set(ObjectPtr<IResourceView> resource);

	};

	/// \brief Base interface for computation outputs.
	/// The class is used to bind a computation output.
	/// \author Raffaele D. Facendola
	class IComputationOutput : public Object{

	public:

		/// \brief Virtual destructor.
		virtual ~IComputationOutput() = 0;

		/// \brief Bind a new resource to the computation.
		/// \param resource Read/write view of the resource to bind to the computation.
		virtual void Set(ObjectPtr<IResourceRWView> resource);

	};

	/////////////////////////////////// ICOMPUTATION PARAMETER ///////////////////////////////////

	template <typename TParameter>
	inline void IComputationParameter::Set(const TParameter& value){

		Set(std::addressof(value),
			sizeof(TParameter));

	}

	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IComputation::CompileFromFile::GetCacheKey() const{

		return ::hash::fnv_1{}(to_string(file_name));

	}

}