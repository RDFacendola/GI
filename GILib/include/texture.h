/// \file texture.h
/// \brief This file contains the interfaces used to define texture resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

#include "gilib.h"
#include "fnv1.h"
#include "resources.h"

namespace gi_lib{

	using std::wstring;
	
	template <typename TObject>
	class ObjectPtr;

	/// \brief Base interface for plain textures.
	/// \author Raffaele D. Facendola.
	class ITexture2D : public IResource{

	public:

		/// \brief Cached structure used to load a texture 2D from file.	
		/// The texture is guaranteed to be read-only.
		struct FromFile{

			USE_CACHE;

			wstring file_name;		///< \brief Name of the file to load.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Interface destructor.
		virtual ~ITexture2D(){}
		
		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMipCount() const = 0;

		/// \brief Get a read-only view to this resource.
		/// \return Returns a pointer to the read-only resource view.
		virtual ObjectPtr<IResourceView> GetView() const = 0;
		
	};

	////////////////////////////// TEXTURE 2D :: FROM FILE ///////////////////////////////

	inline size_t ITexture2D::FromFile::GetCacheKey() const{

		return ::hash::fnv_1{}(to_string(file_name));

	}

}