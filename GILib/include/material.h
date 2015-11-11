/// \file material.h
/// \brief This file defines the interfaces needed to handle materials.
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

	/// \brief Base interface for materials.
	/// \author Raffaele D. Facendola
	class IMaterial : public IResource{

	public:

		/// \brief Structure used to compile a material from a file.
		struct CompileFromFile{

			USE_CACHE;

			std::wstring file_name;			///< \brief Name of the file containing the material code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};
		
		/// \brief Virtual destructor.
		virtual ~IMaterial(){};

		/// \brief Set a texture resource as an input for the material.
		/// The GPU may only read from the specified texture.
		/// \param tag Tag of the input texture to set.
		/// \param texture_2D Pointer to the 2D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2D>& texture_2D) = 0;

		/// \brief Set a texture resource array as an input for the material.
		/// The GPU may only read from the specified texture array.
		/// \param tag Tag of the input texture array to set.
		/// \param texture_2D_array Pointer to the 2D texture array to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture2DArray>& texture_2D_array) = 0;
		
		/// \brief Set a sampler state as an input for the material.
		/// The GPU may only read from the specified sampler state.
		/// \param tag Tag of the input sampler to set.
		/// \param sampler Pointer to the sampler state to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ISampler>& sampler) = 0;

		/// \brief Set a structure resource as an input for the material.
		/// The GPU may only read from the specified structure.
		/// \param tag Tag of the input structure to set.
		/// \param structured_buffer Pointer to the structured buffer to bind.
		/// \param type Concrete type of the buffer.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredBuffer>& structured_buffer) = 0;

		/// \brief Set an array resource as an input for the material.
		/// The GPU may only read from the specified array.
		/// \param tag Tag of the input array to set.
		/// \param structured_array Pointer to the structured array to bind.
		/// \param type Concrete type of the array elements.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<IStructuredArray>& structured_array) = 0;
		
		/// \brief Create a new copy of this material.
		/// Use this material to create a new material instance which shares common states with other instances.
		/// \return Returns a pointer to the new material instance.
		virtual ObjectPtr<IMaterial> Instantiate() = 0;

	};

	////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

	inline size_t IMaterial::CompileFromFile::GetCacheKey() const{

		return gi_lib::Tag(file_name);

	}

}

