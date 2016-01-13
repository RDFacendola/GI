/// \file fx_lighting.h
/// \brief This file contains post process effects that affects lighting such as bloom, glow and color grading.
///
/// \author Raffaele D. Facendola

#pragma once

#include "texture.h"
#include "object.h"
#include "gimath.h"
#include "render_target.h"
#include "resources.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to suppress color whose luminance falls under a threshold.
		/// \author Raffaele D. Facendola
		class FxBrightPass : public IResource {

		public:

			/// \brief Parameters needed by the post processing shader.
			struct Parameters {

				NO_CACHE;

				float threshold_;					///< \brief Exposure offset removed from the auto exposure value.

				float key_value_;					///< \brief Target average luminance of the scene. Used to calculate the auto exposure value.

				float average_luminance_;			///< \brief Current average luminance of the scene. Used to calculate the auto exposure value.

			};

			/// \brief Set the threshold below of which colors are suppressed.
			virtual void SetThreshold(float threshold) = 0;
			
			/// \brief Set the multiplicative factor.
			virtual void SetKeyValue(float key_value) = 0;

			/// \brief Set the average linear luminance of the current frame.
			virtual void SetAverageLuminance(float average_luminance) = 0;

			/// \brief Performs a Gaussian blur of the specified texture.
			/// \param source Texture to blur.
			/// \param destination Destination texture containing the result.
			virtual void Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) = 0;

		};

		/// \brief This class is used to perform a bloom filter.
		/// \author Raffaele D. Facendola
		class FxBloom : public IResource {

		public:

			/// \brief Parameters needed by the post processing shader.
			struct Parameters {

				NO_CACHE;

				float threshold_;					///< \brief Exposure offset removed from the auto exposure value. Used to calculate the glowing parts of the scene.

				float sigma_;						///< \brief Sigma used to calculate the Gaussian kernel.

				float key_value_;					///< \brief Target average luminance of the scene. Used to calculate the auto exposure value.

				float average_luminance_;			///< \brief Current average luminance of the scene. Used to calculate the auto exposure value.

				float strength_;					///< \brief Bloom strength. Higher values yield a brighter scene.

			};

			/// \brief Set the minimum brightness needed for a color to be considered as "glowing".
			/// \param min_brightness Minimum brightness
			virtual void SetThreshold(float threshold) = 0;

			/// \brief Get the sigma used to compute the Gaussian blur kernel.
			virtual float GetSigma() const = 0;

			/// \brief Set the sigma used to compute the Gaussian blur kernel.
			/// \param sigma The new sigma.
			virtual void SetSigma(float sigma) = 0;

			/// \brief Set the multiplicative factor.
			virtual void SetKeyValue(float key_value) = 0;

			/// \brief Set the average linear luminance of the current frame.
			virtual void SetAverageLuminance(float average_luminance) = 0;
			
			/// \brief Set the bloom strength.
			virtual void SetBloomStrength(float strength) = 0;

			/// \brief Process the source image using a bloom filter.
			/// \param source Source image.
			/// \param destination Destination image containing the processed image.
			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) = 0;

		};

		/// \brief Performs the tonemapping of an image.
		/// \author Raffaele D. Facendola
		class FxTonemap : public IResource {

		public:
			
			/// \brief Parameters needed by the post processing shader.
			struct Parameters {

				NO_CACHE;

				float vignette_;					///< \brief Strength of the vignette effect.

				float key_value_;					///< \brief Target average luminance of the scene. Used to calculate the auto exposure value.

				float average_luminance_;			///< \brief Current average luminance of the scene. Used to calculate the auto exposure value.

			};

			/// \brief Set the vignette factor.
			virtual void SetVignette(float vignette) = 0;

			/// \brief Set the multiplicative factor.
			virtual void SetKeyValue(float key_value) = 0;

			/// \brief Set the average linear luminance of the current frame.
			virtual void SetAverageLuminance(float average_luminance) = 0;
			
			/// \brief Performs a tonemapping of the given image.
			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) = 0;
			
		};

	}

}