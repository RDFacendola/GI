/// \file texture.h
/// \brief This file contains the interfaces used to define texture resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

#include "tag.h"

#include "resources.h"

namespace gi_lib{

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

			std::wstring file_name;		///< \brief Name of the file to load.

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
		virtual unsigned int GetMIPCount() const = 0;
		
	};

	/// \brief Base interface for general-purpose textures.
	/// A general-purpose resource can be accessed by the GPU for both reading and writing purposes.
	/// \author Raffaele D. Facendola.
	class IGPTexture2D : ITexture2D{

	public:

		/// \brief Abstract destructor.
		virtual ~IGPTexture2D() = 0 {}

	};

}

////////////////////////////// TEXTURE 2D :: FROM FILE ///////////////////////////////

inline size_t gi_lib::ITexture2D::FromFile::GetCacheKey() const{

	return gi_lib::Tag(file_name);

}