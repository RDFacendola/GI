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

		static const Tag kDiffuseMap;			///< \brief Generic tag used to identify a diffuse map inside a shader. This may not be the actual name!

		static const Tag kSpecularMap;			///< \brief Generic tag used to identify a specular map inside a shader. This may not be the actual name!

		static const Tag kNormalMap;			///< \brief Generic tag used to identify a normal map inside a shader. This may not be the actual name!

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
		
		/// \brief Get an input 2D texture resource.
		/// \param tag Tag of the input tag to get.
		/// \param texture_2D If the method succeeds, it contains a pointer to the requested 2D texture, otherwise it contains nullptr. Output.
		/// \return Returns true if the provided tag was a valid 2D texture, returns false otherwise.
		virtual bool GetInput(const Tag& tag, ObjectPtr<ITexture2D>& texture_2D) const = 0;

		/// \brief Set a texture resource as an input for the material.
		/// The GPU may only read from the specified texture.
		/// \param tag Tag of the input texture to set.
		/// \param texture_3D Pointer to the 3D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<ITexture3D>& texture_3D) = 0;
		
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
		
		/// \brief Set a general-purpose structured array as input for the material.
		/// The GPU has read-only permissions.
		/// \param tag Tag of the input general purpose structured array to set.
		/// \param gp_structured_array Pointer to the general purpose structured array to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetInput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array) = 0;

		/// \brief Set a general-purpose structured array as input/output for the material.
		/// The GPU has both read and write permissions.
		/// \param tag Tag of the input/output scratch structured array to set.
		/// \param scratch_structured_array Pointer to the scratch structured array to bind.
		/// \param keep_initial_count Whether the initial count of the buffer should be kept. This has a meaning only if the structured array is an Append/Consume buffer.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPStructuredArray>& gp_structured_array, bool keep_initial_count = true) = 0;
		
		/// \brief Set a texture resource as an input/output for the material.
		/// The GPU has both read and write permissions.
		/// \param tag Tag of the input/output texture to set.
		/// \param gp_texture_3D Pointer to the general-purpose 3D texture to bind.
		/// \return Returns true if the resource was set successfully, returns false otherwise.
		virtual bool SetOutput(const Tag& tag, const ObjectPtr<IGPTexture3D>& gp_texture_3D) = 0;

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

