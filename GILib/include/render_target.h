/// \file render_target.h
/// \brief This file contains the interfaces used to define render target resources.
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include <vector>

#include "resources.h"
#include "texture.h"

namespace gi_lib{

	class ITexture2D;
	class ITexture2DArray;

	template <typename TObject>
	class ObjectPtr;

	/// \brief Base interface for render targets.
	/// A render target may contain multiple render surfaces as well as a depth-stencil buffer (optional).
	/// \author Raffaele D. Facendola.
	class IRenderTarget : public IResource{

	public:

		/// \brief Structure used to create an empty render target from an explicit description.
		/// \author Raffaele D. Facendola.
		struct FromDescription {

			NO_CACHE;

			unsigned int width;						///< \brief Width of the most detailed level of the texture.

			unsigned int height;					///< \brief Height of the most detailed level of the texture.

			std::vector<TextureFormat> format;		///< \brief Format of each render target surface.

			bool depth;								///< \brief Whether the render target should have a depth buffer.

		};

		virtual ~IRenderTarget(){};

		/// \brief Get the number of surfaces in this render target.
		/// \return Returns the number of surfaces in this render target.
		virtual size_t GetCount() const = 0;

		/// \brief Get a texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual ObjectPtr<ITexture2D> operator[](size_t index) = 0;

		/// \brief Get a texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual ObjectPtr<const ITexture2D> operator[](size_t index) const = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual ObjectPtr<ITexture2D> GetDepthBuffer() = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual ObjectPtr<const ITexture2D> GetDepthBuffer() const = 0;

		/// \brief Resize the render target surfaces.
		/// \param width The new width of the buffer.
		/// \param height The new height of the buffer.
		/// \return Returns true if the texture was resized, returns false otherwise.
		virtual bool Resize(unsigned int width, unsigned int height) = 0;

		/// \brief Get the width of the render target in pixels.
		/// \return Returns the width of the render target.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the render target in pixels.
		/// \return Returns the height of the render target.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the format of each surface inside the render target.
		/// \return Returns the format of each surface inside the render target.
		virtual std::vector<TextureFormat> GetFormat() const = 0;

	};

	/// \brief Base interface for render-target cache.
	/// \author Raffaele D. Facendola.
	class IRenderTargetCache : public IResource {

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
		virtual void PushToCache(const ObjectPtr<IRenderTarget>& texture) = 0;

		/// \brief Pops a texture matching the specified values from the cache.
		/// \param width Width of the requested texture.
		/// \param height Height of the requested texture.
		/// \param format Format of the requested texture.
		/// \param has_depth Whether the render target should have a depth.
		/// \param generate Whether to generate a brand new texture if none can be found.
		/// \return Returns a pointer to a cached texture meeting the specified requirements if any.
		/// \remarks If generate is set to "true" this method is guaranteed to return a texture.
		virtual ObjectPtr<IRenderTarget> PopFromCache(unsigned int width, unsigned int height, std::vector<TextureFormat> format, bool has_depth, bool generate = true) = 0;

	};

	/// \brief Base interface for render target arrays.
	/// A render target array is an array of textures, each of which can be drawn upon.
	/// Elements in the array are guaranteed to have the same dimensions.
	/// The array also defines an optional shared depth stencil buffer for Z testing while drawing.
	/// \author Raffaele D. Facendola
	class IRenderTargetArray : public IResource {
		
	public:

		/// \brief Structure used to create an empty render target array from an explicit description.
		/// \author Raffaele D. Facendola.
		struct FromDescription {

			NO_CACHE;

			unsigned int width;			///< \brief Width of the most detailed level of the texture.

			unsigned int height;		///< \brief Height of the most detailed level of the texture.

			unsigned int count;			///< \brief Elements inside the array

			TextureFormat format;		///< \brief Format of the texture.

		};

		virtual ~IRenderTargetArray() {};

		/// \brief Get the number of elements in the array.
		/// \return Returns the number of elements in the array.
		virtual size_t GetCount() const = 0;

		/// \brief Get the texture associated to the render target array.
		/// \return Returns the texture associated to the render target array.
		virtual ObjectPtr<ITexture2DArray> GetRenderTargets() = 0;

		/// \brief Get the texture associated the the render target array.
		/// \return Returns the texture associated to the render target array.
		virtual ObjectPtr<const ITexture2DArray> GetRenderTargets() const = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target, if any.
		virtual ObjectPtr<ITexture2D> GetDepthBuffer() = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target, if any.
		virtual ObjectPtr<const ITexture2D> GetDepthBuffer() const = 0;

		/// \brief Get the width of one of the render targets in pixels.
		/// \return Returns the width of one of the render targets.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of one of the render targets in pixels.
		/// \return Returns the height of one of the render targets.
		virtual unsigned int GetHeight() const = 0;

	};

	////////////////////////////// RENDER TARGET CACHE :: SINGLETON ///////////////////////////////

	inline size_t IRenderTargetCache::Singleton::GetCacheKey() const {

		return gi_lib::Tag("Singleton");

	}

}

#endif