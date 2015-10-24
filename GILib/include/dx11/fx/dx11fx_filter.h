/// \file dx11fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_filter.h"

#include "..\dx11material.h"

#include "..\..\tag.h"

namespace gi_lib {

	namespace dx11 {

		namespace fx {

			/// \brief This class is used to suppress color whose brightness falls under a given threshold.
			/// \author Raffaele D. Facendola
			class DX11FxBrightPass : public gi_lib::fx::FxHighPass {

			public:

				/// \brief Create a new High-pass filter.
				/// \param threshold Threshold below of which the colors are suppressed.
				DX11FxBrightPass(float threshold);

				virtual float GetThreshold() const override;

				virtual void SetThreshold(float threshold) override;

				virtual void Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;

			private:

				/// \brief Constant buffer used to pass the parameters to the filtering shader.
				struct Parameters {

					float gThreshold;										///< \brief Brightness threshold below of which colors are suppressed.

				};

				static const Tag kSourceTexture;							///< \brief Tag of the source texture to filter.

				static const Tag kSampler;									///< \brief Tag of the sampler used to sample the source texture.

				static const Tag kParameters;								///< \brief Tag of the constant buffer used to pass parameters to the shader.

				ObjectPtr<DX11Material> filter_shader_;						///< \brief Shader performing the scaling.

				ObjectPtr<DX11Sampler> sampler_;							///< \brief Sampler used to sample the source texture.

				ObjectPtr<DX11StructuredBuffer> parameters_;				///< \brief Parameters used to perform the filtering.

				float threshold_;											///< \brief Variance of the Gaussian function.

			};

			////////////////////////////////// DX11 FX HIGH PASS ////////////////////////////////

			inline float DX11FxBrightPass::GetThreshold() const {

				return threshold_;

			}

		}

	}

}