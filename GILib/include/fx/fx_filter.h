/// \file fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"

#include "texture.h"
#include "render_target.h"
#include "resources.h"
#include "gimath.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to perform a Gaussian blur to a texture using the GPU.
		/// \author Raffaele D. Facendola
		class FxGaussianBlur : public IResource {

		public:

			/// \brief Parameters needed by the post processing shader.
			struct Parameters {

				NO_CACHE;

				float sigma_;					///< \brief Sigma used to compute the Gaussian kernel.

				unsigned int kernel_radius_;	///< \brief Half size of the Gaussian kernel.

			};

			/// \brief Get the sigma used to compute the blur kernel.
			virtual float GetSigma() const = 0;

			/// \brief Set the sigma used to compute the blur kernel.
			virtual void SetSigma(float sigma) = 0;

			/// \brief Performs a Gaussian blur of the specified texture.
			/// \param source Texture to blur.
			/// \param destination Destination texture containing the result.
			/// \param offset Top-left corner inside the destination texture where the result will be stored.
			virtual void Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination, const Vector2i& offset) = 0;
			
		};
	
	}

}