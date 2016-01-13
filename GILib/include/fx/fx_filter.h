/// \file fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include <sstream>

#include "object.h"

#include "texture.h"
#include "render_target.h"
#include "resources.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to calculate the luminance of an image.
		/// \author Raffaele D. Facendola
		class FxLuminance : public IResource{

		public:

			/// \brief Parameters needed by the post processing shader.
			struct Parameters {

				NO_CACHE;

				float min_luminance_;				///< \brief Threshold below of which the luminance is clamped to.

				float max_luminance_;				///< \brief Threshold above of which the luminance is clamped to.

				float low_percentage_;				///< \brief Lowest percentile used to calculate the average luminance of the scene.

				float high_percentage_;				///< \brief Highest percentile used to calculate the average luminance of the scene

			};

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