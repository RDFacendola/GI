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

	/// \brief Base interface for sampler states.
	/// \author Raffaele D. Facendola.
	class ISampler : public IResource{

	public:

		/// \brief Cached structure used to create a sampler state from a plain description.
		struct FromDescription{

			USE_CACHE;

			/// \brief Texture mapping.
			TextureMapping texture_mapping;

			/// \brief Anisotropy level.
			unsigned int anisotropy_level;

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

	};

	/////////////////////////////// ISAMPLER :: FROM DESCRIPTION ///////////////////////////////

	inline size_t ISampler::FromDescription::GetCacheKey() const{

		// | ... | texture_mapping | anisotropy_level |
		//      40                 8                  0

		return (anisotropy_level & 0xFF) | (static_cast<unsigned int>(texture_mapping) << 8);

	}

}