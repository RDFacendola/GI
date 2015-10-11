/// \file fx_blur.h
/// \brief This file contains classes used to blur a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"

#include "texture.h"
#include "render_target.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to perform a Gaussian blur to a texture using the GPU.
		/// \author Raffaele D. Facendola
		class FxGaussianBlur {

		public:

			static const int kKernelSize = 11;			///< \brief Size of the kernel.

			static const int kBlurRadius = 5;			///< \brief Blur radius.

			/// \brief Get the sigma used to compute the blur kernel.
			virtual float GetSigma() const = 0;

			/// \brief Set the sigma used to compute the blur kernel.
			virtual void SetSigma(float sigma) = 0;

			/// \brief Performs a Gaussian blur of the specified texture.
			/// \param source Texture to blur.
			/// \param destination Destination texture containing the result.
			virtual void Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) = 0;

		};
		
	}

}