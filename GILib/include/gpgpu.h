/// \file gpgpu.h
/// \brief This file contains the interfaces for general-purpose computing on GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeindex>
#include <string>
#include "resources.h"
#include "tag.h"
#include "object.h"

#include "texture.h"
#include "buffer.h"
#include "sampler.h"

namespace gi_lib{
	
	/// \brief Base interface for GPU computations.
	/// \author Raffaele D. Facendola
	class IComputation : public IResource{

	public:

		/// \brief Structure used to compile a compute shader from a file.
		struct CompileFromFile{

			USE_CACHE;

			std::wstring file_name;			///< \brief Name of the file containing the compute shader code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Virtual destructor.
		virtual ~IComputation(){};
		
		/// \brief Set a texture resource as an input for the current computation.
		/// The GPU may only read from the specified texture.
		/// \param tag Tag of the input texture to set.
		/// \param texture_2D Pointer to the 2D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) = 0;

		/// \brief Set a sampler state as an input for the current computation.
		/// The GPU may only read from the specified sampler state.
		/// \param tag Tag of the input texture to set.
		/// \param sampler Pointer to the sampler state to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler_state) = 0;
		
		/// \brief Set a structure resource as an input for the current computation.
		/// The GPU may only read from the specified structure.
		/// \tparam TType Concrete type of the structure.
		/// \param tag Tag of the input structure to set.
		/// \param structured_buffer Pointer to the structured buffer to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		template <typename TType>
		bool SetInput(const Tag& tag, const ObjectPtr<StructuredBuffer<TType>>& structured_buffer);

		/// \brief Set an array resource as an input for the current computation.
		/// The GPU may only read from the specified array.
		/// \tparam TElement Type of the array's elements.
		/// \param tag Tag of the input array to set.
		/// \param structured_array Pointer to the structured array to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		template <typename TElement>
		bool SetInput(const Tag& tag, const ObjectPtr<StructuredArray<TElement>>& structured_array);

		/// \brief Set a texture resource as an input/output for the current computation.
		/// The GPU has both read and write permissions.
		/// \param tag Tag of the input/output texture to set.
		/// \param gp_texture_2D Pointer to the general-purpose 2D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture2D>& gp_texture_2D) = 0;

		/// \brief Set a structure resource as an input for the computation.
		/// The GPU may only read from the specified structure.
		/// \param tag Tag of the input structure to set.
		/// \param structured_buffer Pointer to the structured buffer to bind.
		/// \param type Concrete type of the buffer.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) = 0;

		/// \brief Set an array resource as an input for the computation.
		/// The GPU may only read from the specified array.
		/// \param tag Tag of the input array to set.
		/// \param structured_array Pointer to the structured array to bind.
		/// \param type Concrete type of the array elements.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) = 0;

	};

	////////////////////////////// ICOMPUTATION ////////////////////////////////////////////////
	
	template <typename TType>
	inline bool IComputation::SetInput(const Tag& tag, const ObjectPtr<StructuredBuffer<TType>>& structured_buffer){

		return SetInput(tag,
						structured_buffer->GetBuffer());

	}

	template <typename TElement>
	inline bool IComputation::SetInput(const Tag& tag, const ObjectPtr<StructuredArray<TElement>>& structured_array){

		return SetInput(tag,
						structured_array->GetBuffer());

	}

	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IComputation::CompileFromFile::GetCacheKey() const{

		return gi_lib::Tag(file_name);

	}	

}


