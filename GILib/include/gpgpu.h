/// \file gpgpu.h
/// \brief This file contains the interfaces for general-purpose computing on GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{

	/// \brief Base interface for computation that can be executed on the GPU.
	/// \author Raffaele D. Facendola
	class IGPUComputation : public IResource{

		/// \brief Structure used to compile a compute shader from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the compute shader code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Virtual destructor.
		virtual ~IGPUComputation(){}

		/// \brief Execute the GPU program dispatching a grid of threads.
		/// The execution of the threads is guaranteed to be synchronous.
		/// \param x Threads to dispatch along the X-axis.
		/// \param y Threads to dispatch along the Y-axis.
		/// \param z Threads to dispatch along the Z-axis.
		/// \remarks The total amount of dispatched threads is x*y*z.
		/// \remarks The method will take care of dispatching the correct amount of thread group, based on the specified number of threads.
		virtual void Dispatch(unsigned int x, unsigned int y, unsigned int z) = 0;

		//virtual ObjectPtr<IVariable> GetVariable(const string& name) = 0;

		//virtual ObjectPtr<IVariable> GetResource(const string& name) = 0;		// Input, Read

		//virtual ObjectPtr<IVariable> GetRWResource(const string& name) = 0;		// Input/Output, Read/Write

	};

}