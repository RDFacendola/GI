/// \file sampler.h
/// \brief This file contains the interfaces used to define sampler states.
///
/// \author Raffaele D. Facendola

#pragma once

#include "tag.h"

#include "resources.h"
#include "graphics.h"

namespace gi_lib{

	template <typename TObject>
	class ObjectPtr;

	/// \brief Describes how texture coordinates are mapped.
	/// Samplers use this information to resolve texture coordinates outside the [0;1] boundaries.
	enum class TextureMapping : unsigned int{

		WRAP,				///< \brief Repeat the texture for texture coordinates outside the boundary [0;1] every integer.
		CLAMP,				///< \brief Texture coordinates below 0 or above 1 are set to 0 and 1 instead.
		COLOR,				///< \brief Texture coordinates below 0 or above 1 sample a predefined color.

	};

	/// \brief Describes how the texture should be filtered while sampling int.
	enum class TextureFiltering : unsigned int {

		NEAREST,			///< \brief The nearest pixel is sampled without filtering.
		BILINEAR,			///< \brief Bilinear interpolation on the nearest mipmap level.
		TRILINEAR,			///< \brief Bilinear interpolation on the two nearest mipmap levels with an additional interpolation between the two results.
		ANISOTROPIC,		///< \brief Orientation-corrected interpolation.
		PERCENTAGE_CLOSER,	///< \brief Percentage closer filtering.

	};

	/// \brief Base interface for sampler states.
	/// \author Raffaele D. Facendola.
	class ISampler : public IResource{

	public:

		/// \brief Cached structure used to create a sampler state from a plain description.
		struct FromDescription{

			USE_CACHE;

			/// \brief Texture mapping.
			TextureMapping texture_mapping;

			/// \brief How the texture should be filtered.
			TextureFiltering texture_filtering;

			/// \brief Anisotropy level, used only when texture_filtering is "ANISOTROPIC".
			unsigned int anisotropy_level;

			/// \brief Border color, used only when texture_mapping is "COLOR".
			Color default_color;

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};
		
		/// \brief Interface destructor.
		virtual ~ISampler(){}

		/// \brief Get the maximum anisotropy level.
		/// \return Returns the maximum anisotropy level.
		virtual unsigned int GetMaxAnisotropy() const = 0;

		/// \brief Get the texture mapping along each dimension.
		/// \return Returns the texture mapping along each dimension.
		virtual TextureMapping GetTextureMapping() const = 0;

		/// \brief Get the texture filtering mode.
		/// \return Returns the texture filtering mode.
		virtual TextureFiltering GetTextureFiltering() const = 0;

	};

	/////////////////////////////// ISAMPLER :: FROM DESCRIPTION ///////////////////////////////

	inline size_t ISampler::FromDescription::GetCacheKey() const{

		// | ... | texture_filtering | texture_mapping | anisotropy_level |
		//      32                   16                8                  0

		return (anisotropy_level & 0xFF) | 
			   (static_cast<unsigned int>(texture_mapping) << 8) |
			   (static_cast<unsigned int>(texture_filtering) << 16);

	}

}