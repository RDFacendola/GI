/// \file fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"

#include "texture.h"
#include "render_target.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to suppress color under a threshold.
		/// \author Raffaele D. Facendola
		class FxHighPass {

		public:

			/// \brief Get the sigma below of which colors are suppressed.
			virtual float GetThreshold() const = 0;

			/// \brief Set the threshold below of which colors are suppressed.
			virtual void SetThreshold(float threshold) = 0;

			/// \brief Performs a Gaussian blur of the specified texture.
			/// \param source Texture to blur.
			/// \param destination Destination texture containing the result.
			virtual void Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) = 0;

		};

	}

}