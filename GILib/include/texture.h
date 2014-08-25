/// \file texture.h
/// \brief Interfaces and methods for texture management.
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{

	/// \brief Techniques used to resolve texture coordinates that are outside of the texture's boundaries.
	enum class WrapMode{

		WRAP,		///< Texture coordinates are repeated with a period of 1.
		CLAMP		///< Texture coordinates are clamped inside the range [0, 1]

	};

	/// \brief Base interface for plain textures.
	/// \author Raffaele D. Facendola.
	class Texture2D : public Resource{

	public:

		virtual ~Texture2D(){}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual size_t GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual size_t GetHeight() const = 0;

		/// \brief Get the mip map level count.
		/// \return Returns the mip map level count.
		virtual unsigned int GetMipMapCount() const = 0;

		/// \brief Get the wrap mode.

		/// \see WrapMode
		/// \return Return the wrap mode used by this texture.
		virtual WrapMode GetWrapMode() const = 0;

		/// \brief Set the wrap mode.
		/// \param wrap_mode Wrap mode to use.
		virtual void SetWrapMode(WrapMode wrap_mode) = 0;

	};

}
