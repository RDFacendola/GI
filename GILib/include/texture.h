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

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() = 0;

		/// \brief Get the mip map level count.
		/// \return Returns the mip map level count.
		virtual unsigned int GetMipMapCount() = 0;

		/// \brief Get the wrap mode.

		/// \see WrapMode
		/// \return Return the wrap mode used by this texture.
		virtual WrapMode GetWrapMode() = 0;

		/// \brief Set the wrap mode.
		/// \param wrap_mode Wrap mode to use.
		virtual void SetWrapMode(WrapMode wrap_mode) = 0;

		/// \brief Get the maximum anisotropy level.
		/// \return Return the max anisotropy level for this texture.
		virtual unsigned int GetMaxAnisotropy() = 0;

		/// \brief Set the maximum anisotropy level.

		/// The effective anisotropy level that will be used may be different, according to the adapter's capabilities and context options.
		/// \param max_anisotropy The maximum level of anisotropy. 
		virtual void SetMaxAnisotropy(unsigned int max_anisotropy) = 0;

		/// \brief Get the upper end of the mipmap range
		/// \return Returns the upper end of the mipmap range
		virtual unsigned int GetMaxLOD() = 0;

		/// \brief Set the upper end of the mipmap range. 
		/// \param max_LOD The maximum LOD level to use. Set this to 0 to disable the mip map entirely.
		virtual void SetMaxLOD(unsigned int max_LOD) = 0;

	};

}
