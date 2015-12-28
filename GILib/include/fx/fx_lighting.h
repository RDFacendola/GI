/// \file fx_lighting.h
/// \brief This file contains post process effects that affects lighting such as bloom, glow and color grading.
///
/// \author Raffaele D. Facendola

#pragma once

#include "texture.h"
#include "object.h"
#include "gimath.h"
#include "render_target.h"

namespace gi_lib {

	namespace fx {

		/// \brief This class is used to suppress color whose luminance falls under a threshold.
		/// \author Raffaele D. Facendola
		class FxBrightPass {

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

		/// \brief This class is used to perform a bloom filter.
		/// \author Raffaele D. Facendola
		class FxBloom {

		public:

			/// \brief Get the minimum brightness needed for a color to be considered as "glowing".
			virtual float GetMinBrightness() const = 0;

			/// \brief Set the minimum brightness needed for a color to be considered as "glowing".
			/// \param min_brightness Minimum brightness
			virtual void SetMinBrightness(float min_brightness) = 0;

			/// \brief Get the sigma used to compute the Gaussian blur kernel.
			virtual float GetSigma() const = 0;

			/// \brief Set the sigma used to compute the Gaussian blur kernel.
			/// \param sigma The new sigma.
			virtual void SetSigma(float sigma) = 0;

			/// \brief Get the scaling of the blurred surface.
			virtual Vector2f GetBlurScaling() const = 0;

			/// \brief Set the scaling of the blurred surface.
			/// \param scaling The new scaling of the blurred surface.
			virtual void SetBlurScaling(const Vector2f& scaling) = 0;
			
			/// \brief Process the source image using a bloom filter.
			/// \param source Source image.
			/// \param destination Destination image containing the processed image.
			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) = 0;

		};

		/// \brief Performs the tonemapping of an image.
		/// \author Raffaele D. Facendola
		class FxTonemap {

		public:
			
			/// \brief Get the vignette factor.
			virtual float GetVignette() const = 0;

			/// \brief Set the vignette factor.
			virtual void SetVignette(float vignette) = 0;

			/// \brief Get the key value of the image.
			/// Think of this value as desired average luminance or general "mood".
			virtual float GetKeyValue() const = 0;

			/// \brief Set the multiplicative factor.
			virtual void SetKeyValue(float key_value) = 0;

			/// \brief Performs a tonemapping of the given image.
			/// \param average_luminance Average luminance of the current frame stored as 1x1 texture.
			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<ITexture2D>& average_luminance, const ObjectPtr<IGPTexture2D>& destination) = 0;
			
		};

	}

}