/// \file material.h
/// \brief This file defines the interfaces needed to handle materials.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeindex>

#include "resources.h"
#include "tag.h"
#include "object.h"

#include "texture.h"
#include "buffer.h"

namespace gi_lib{

	/// \brief Base interface for materials.
	/// \author Raffaele D. Facendola
	class IMaterial : public IResource{

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

			ObjectPtr<IMaterial> base;	///< \brief Material to instantiate.

		};

		/// \brief Virtual destructor.
		virtual ~IMaterial(){};

		/// \brief Set a texture resource as an input for the current computation.
		/// The GPU may only read from the specified texture.
		/// \param tag Tag of the input texture to set.
		/// \param texture_2D Pointer to the 2D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, ObjectPtr<ITexture2D> texture_2D) = 0;

		/// \brief Set a structure resource as an input for the current computation.
		/// The GPU may only read from the specified structure.
		/// \tparam TType Concrete type of the structure.
		/// \param tag Tag of the input structure to set.
		/// \param structured_buffer Pointer to the structured buffer to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		template <typename TType>
		bool SetInput(const Tag& tag, ObjectPtr<StructuredBuffer<TType>> structured_buffer);

		/// \brief Set an array resource as an input for the current computation.
		/// The GPU may only read from the specified array.
		/// \tparam TElement Type of the array's elements.
		/// \param tag Tag of the input array to set.
		/// \param structured_array Pointer to the structured array to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		template <typename TElement>
		bool SetInput(const Tag& tag, ObjectPtr<StructuredArray<TElement>> structured_array);

	private:

		/// \brief Set a structure resource as an input for the current computation.
		/// The GPU may only read from the specified structure.
		/// \param tag Tag of the input structure to set.
		/// \param structured_buffer Pointer to the structured buffer to bind.
		/// \param type Concrete type of the buffer.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, ObjectPtr<IStructuredBuffer> structured_buffer, std::type_index type) = 0;

		/// \brief Set an array resource as an input for the current computation.
		/// The GPU may only read from the specified array.
		/// \param tag Tag of the input array to set.
		/// \param structured_array Pointer to the structured array to bind.
		/// \param type Concrete type of the array elements.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, ObjectPtr<IStructuredArray> structured_array, std::type_index type) = 0;
		
	};

}

////////////////////////////// MATERIAL :: COMPILE FROM FILE ///////////////////////////////

inline size_t gi_lib::IMaterial::CompileFromFile::GetCacheKey() const{

	return gi_lib::Tag(file_name);

}

////////////////////////////// IMATERIAL ////////////////////////////////////////////////

template <typename TType>
inline bool gi_lib::IMaterial::SetInput(const Tag& tag, ObjectPtr<StructuredBuffer<TType>> structured_buffer){

	return SetInput(tag,
					structured_buffer,
					std::type_index(typeid(TType)));

}

template <typename TElement>
inline bool gi_lib::IMaterial::SetInput(const Tag& tag, ObjectPtr<StructuredArray<TElement>> structured_array){

	return SetInput(tag,
					structured_array,
					std::type_index(typeid(TElement)));

}
