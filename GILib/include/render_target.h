/// \file render_target.h
/// \brief This file contains the interfaces used to define render target resources.
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "resources.h"

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

	};

	/// \brief Base interface for render target arrays.
	/// A render target array is an array of textures, each of which can be drawn upon.
	/// Elements in the array are guaranteed to have the same dimensions.
	/// The array also defines an optional shared depth stencil buffer for Z testing while drawing.
	/// \author Raffaele D. Facendola
	class IRenderTargetArray : public IResource {
		
	public:

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

}

#endif