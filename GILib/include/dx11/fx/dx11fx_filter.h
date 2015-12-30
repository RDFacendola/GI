/// \file dx11fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_filter.h"

#include "..\dx11gpgpu.h"

#include "..\..\tag.h"

namespace gi_lib {

	namespace dx11 {

		namespace fx {

			/// \brief Gaussian filter.
			class DX11FxLuminance : public gi_lib::fx::FxLuminance {

			public:

				DX11FxLuminance(float min_luminance, float max_luminance, float low_percentage = 0.85f, float high_percentage = 0.9f);

				virtual float ComputeAverageLuminance(const ObjectPtr<ITexture2D>& source) const override;

				virtual void SetMinLuminance(float min_luminance) override;

				virtual void SetMaxLuminance(float max_luminance) override;

				virtual void SetLowPercentage(float low_percentage) override;

				virtual void SetHighPercentage(float high_percentage) override;

			private:

				struct LuminanceHistogramParameters{

					float gLogMinimum;							///< \brief Minimum log luminance.
					float gLogMaximum;							///< \brief Maximum log luminance.

					Vector2f reserved;

				};

				static const unsigned int kBinCount;								///< \brief Bins used for the histogram.

				static const Tag kSourceTexture;									///< \brief Tag of the source texture to blur.

				static const Tag kHistogram;										///< \brief Tag of the histogram.

				static const Tag kParameters;										///< \brief Tag of the shader parameters.

				float low_percentage_;												///< \brief Percentage of samples whose luminance is below the calculated average.
						
				float high_percentage_;												///< \brief Percentage of samples whose luminance is above the calculated average.

				float min_log_luminance_;

				float max_log_luminance_;

				ObjectPtr<DX11Computation> clear_shader_;							///< \brief Used to clear the histogram.

				ObjectPtr<DX11Computation> luminance_shader_;						///< \brief Used to calculate the luminance of the image.

				ObjectPtr<DX11ScratchStructuredArray> log_luminance_histogram_;		///< \brief Contains the histogram of the log luminance.

				ObjectPtr<DX11StructuredBuffer> luminance_parameters_;				///< \brief Parameters passed to the compute shader.

			};

		}

	}

}