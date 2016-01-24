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

	/// \brief Texture surface format.
	enum class TextureFormat : unsigned int{

		HALF,				///< \brief 16-bit format with a single half-precision floating point number.

		RGBA_BYTE_UNORM,	///< \brief 32-bit format with 4 8-bit channels. Each channel stores a byte-sized number in the range [0;1]

		RGBA_HALF,			///< \brief 64-bit format with 4 16-bit channels. Each channel stores a half-precision floating point number.
		RGBA_FLOAT,			///< \brief 128-bit format with 4 32-bit channels. Each channel stores a single-precision floating point number.
		
		
		BGRA_BYTE_UNORM,

		RGB_FLOAT,			///< \brief 32-bit format with 2 11-bit red/green channels and 1 10-bit blue channel.

		RG_HALF,			///< \brief 32-bit format with 2 16-bit channels. Each channel stores a half-precision floating point number.
		RG_FLOAT,			///< \brief 64-bit format with 2 32-bit channels. Each channel stores a single-precision floating point number.

		DEPTH_STENCIL,		///< \brief 32-bit format with 1 24-bit depth channel and 1 8-bit typeless channel.

		BC3_UNORM,			///< \brief Block-compressed texture with interpolated alpha. Also known as DXT4/DXT5.

	};

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

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;
		
	};

	/// \brief Base interface for general-purpose textures.
	/// A general-purpose resource can be accessed by the GPU for both reading and writing purposes.
	/// \author Raffaele D. Facendola.
	class IGPTexture2D : public IResource{

	public:

		/// \brief Structure used to create an empty general-purpose texture 2D from an explicit description.
		/// \author Raffaele D. Facendola.
		struct FromDescription{

			NO_CACHE;

			unsigned int width;			///< \brief Width of the most detailed level of the texture.

			unsigned int height;		///< \brief Height of the most detailed level of the texture.

			unsigned int mips;			///< \brief Total number of MIP levels.

			TextureFormat format;		///< \brief Format of the texture.

		};

		/// \brief Abstract destructor.
		virtual ~IGPTexture2D() = 0 {}

		/// \brief Get the underlying texture.
		/// \return Returns a pointer to the underlying texture.
		virtual ObjectPtr<ITexture2D> GetTexture() = 0;
		
		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMIPCount() const = 0;

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;

	};

	/// \brief Base interface for general-purpose textures cache.
	/// \author Raffaele D. Facendola.
	class IGPTexture2DCache : public IResource {
		
	public:

		/// \brief Singleton
		/// \author Raffaele D. Facendola.
		struct Singleton {

			USE_CACHE;

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Push the specified texture inside the cache and clears out the pointer.
		virtual void PushToCache(const ObjectPtr<IGPTexture2D>& texture) = 0;

		/// \brief Pops a texture matching the specified values from the cache.
		/// \param width Width of the requested texture.
		/// \param height Height of the requested texture.
		/// \param format Format of the requested texture.
		/// \param generate Whether to generate a brand new texture if none can be found.
		/// \return Returns a pointer to a cached texture meeting the specified requirements if any.
		/// \remarks If generate is set to "true" this method is guaranteed to return a texture.
		virtual ObjectPtr<IGPTexture2D> PopFromCache(unsigned int width, unsigned int height, TextureFormat format, bool generate = true) = 0;

	};

	/// \brief Base interface for plain texture arrays.
	/// \author Raffaele D. Facendola.
	class ITexture2DArray : public IResource {

	public:

		/// \brief Structure used to create an empty texture 2D from an explicit description.
		/// \author Raffaele D. Facendola.
		struct FromDescription {

			NO_CACHE;

			unsigned int width;			///< \brief Width of the most detailed level of the texture.

			unsigned int height;		///< \brief Height of the most detailed level of the texture.

			unsigned int count;			///< \brief Elements inside the array

			unsigned int mips;			///< \brief Total number of MIP levels.

			TextureFormat format;		///< \brief Format of the texture.

		};

		/// \brief Interface destructor.
		virtual ~ITexture2DArray() {}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMIPCount() const = 0;

		/// \brief Get the number of elements in the array.
		/// \return Returns the number of elements in the array.
		virtual unsigned int GetCount() const = 0;

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;

	};

	/// \brief Base interface for general-purpose texture arrays.
	/// A general-purpose resource can be accessed by the GPU for both reading and writing purposes.
	/// \author Raffaele D. Facendola.
	class IGPTexture2DArray : public IResource {

	public:

		/// \brief Interface destructor.
		virtual ~IGPTexture2DArray() {}

		/// \brief Get the underlying texture array.
		/// \return Returns a pointer to the underlying texture array.
		virtual ObjectPtr<ITexture2DArray> GetTextureArray() = 0;

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMIPCount() const = 0;

		/// \brief Get the number of elements in the array.
		/// \return Returns the number of elements in the array.
		virtual unsigned int GetCount() const = 0;

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;

	};
	
	/// \brief Base interface for 3D textures.
	/// \author Raffaele D. Facendola.
	class ITexture3D : public IResource {

	public:

		/// \brief Interface destructor.
		virtual ~ITexture3D() {}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the depth of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetDepth() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMIPCount() const = 0;

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;

	};

	/// \brief Base interface for general-purpose 3D textures.
	/// A general-purpose resource can be accessed by the GPU for both reading and writing purposes.
	/// \author Raffaele D. Facendola.
	class IGPTexture3D : public IResource {

	public:

		/// \brief Structure used to create an empty general-purpose texture 2D from an explicit description.
		/// \author Raffaele D. Facendola.
		struct FromDescription {

			NO_CACHE;

			unsigned int width;			///< \brief Width of the most detailed level of the texture.

			unsigned int height;		///< \brief Height of the most detailed level of the texture.

			unsigned int depth;			///< \brief Depth of the most detailed level of the texture.

			unsigned int mips;			///< \brief Total number of MIP levels.

			TextureFormat format;		///< \brief Format of the texture.

		};

		/// \brief Abstract destructor.
		virtual ~IGPTexture3D() = 0 {}

		/// \brief Get the underlying texture.
		/// \return Returns a pointer to the underlying texture.
		virtual ObjectPtr<ITexture3D> GetTexture() = 0;

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the depth of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetDepth() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMIPCount() const = 0;

		/// \brief Get the texture format.
		/// \return Returns the texture format.
		virtual TextureFormat GetFormat() const = 0;

	};
	
}

////////////////////////////// TEXTURE 2D :: FROM FILE ///////////////////////////////

inline size_t gi_lib::ITexture2D::FromFile::GetCacheKey() const{

	return gi_lib::Tag(file_name);

}

////////////////////////////// GP TEXTURE2D CACHE :: SINGLETON ///////////////////////////////

inline size_t gi_lib::IGPTexture2DCache::Singleton::GetCacheKey() const {

	return gi_lib::Tag("Singleton");

}