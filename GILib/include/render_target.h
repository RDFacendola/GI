/// \file render_target.h
/// \brief This file contains the interfaces used to define render target resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{

	class ITexture2D;

	template <typename TObject>
	class ObjectPtr;

	/// \brief Base interface for render targets.
	/// A render target may contain multiple render surfaces as well as a depth-stencil buffer.
	/// \author Raffaele D. Facendola.
	class IRenderTarget : public IResource{

	public:

		virtual ~IRenderTarget(){};

		/// \brief Get the number of surfaces in this render target.
		/// \return Returns the number of surfaces in this render target.
		virtual unsigned int GetCount() const = 0;

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
		virtual ObjectPtr<ITexture2D> GetZStencil() = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual ObjectPtr<const ITexture2D> GetZStencil() const = 0;

		/// \brief Get the aspect ratio of the render target.
		/// The aspect ratio is Width/Height.
		/// \return Returns the aspect ratio of the render target.
		virtual float GetAspectRatio() const = 0;

		/// \brief Get the anti-aliasing mode of the render target.
		/// The anti-aliasing mode influences the number of samples per pixel for techniques like MSAA.
		/// \return Return the anti-aliasing mode of the render target.
		virtual AntialiasingMode GetAntialiasing() const = 0;

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

}