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

		/// \brief This class is used to calculate the luminance of an image.
		/// \author Raffaele D. Facendola
		class FxLuminance {

		public:

			/// \brief Calculate the average relative luminance of the given source image.
			/// \param source Source texture.
			/// \return Returns the average luminance of the given image.
			virtual float ComputeAverageLuminance(const ObjectPtr<ITexture2D>& source) const = 0;

			virtual void SetMinLuminance(float min_luminance) = 0;

			virtual void SetMaxLuminance(float max_luminance) = 0;

			virtual void SetLowPercentage(float low_percentage) = 0;

			virtual void SetHighPercentage(float high_percentage) = 0;

		};

	}

}