/// \file dx11fx_blur.h
/// \brief This file contains classes used to blur a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_blur.h"

#include "..\dx11gpgpu.h"

#include "..\..\tag.h"

namespace gi_lib {

	namespace dx11 {

		namespace fx {

			/// \brief Gaussian filter.
			class DX11FxGaussianBlur: public gi_lib::fx::FxGaussianBlur {

			public:

				/// \brief Create a new Gaussian blur filter.
				/// \param sigma Variance used to compute the blurring kernel.
				DX11FxGaussianBlur(float sigma);
				
				virtual float GetSigma() const override;

				virtual void SetSigma(float sigma) override;

				virtual void Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) override;

				virtual void Blur(const ObjectPtr<ITexture2DArray>& source, const ObjectPtr<IGPTexture2DArray>& destination) override;

			private:

				// Tonemapping

				static const Tag kSourceTexture;							///< \brief Tag of the source texture to blur.

				static const Tag kDestinationTexture;						///< \brief Tag of the destination texture to write.

				static const Tag kBlurKernel;								///< \brief Tag of the kernel used for weighting.

				ObjectPtr<DX11StructuredArray> kernel_;						///< \brief Contains the actual values of the kernel.

				ObjectPtr<DX11Computation> hblur_shader_;					///< \brief Used to perform the horizontal blur pass.

				ObjectPtr<DX11Computation> vblur_shader_;					///< \brief Used to perform the vertical blur pass.

				ObjectPtr<DX11Computation> hblur_array_shader_;				///< \brief Used to perform the horizontal blur pass for texture arrays.

				ObjectPtr<DX11Computation> vblur_array_shader_;				///< \brief Used to perform the vertical blur pass for texture arrays.

				ObjectPtr<DX11GPTexture2D> temp_texture_;					///< \brief Temporary texture containing the first pass of the Gaussian blur.

				ObjectPtr<DX11GPTexture2DArray> temp_texture_array_;		///< \brief Temporary texture array containing the first pass of the Gaussian blur.

				float sigma_;												///< \brief Variance of the Gaussian function.

			};

			///////////////////////////// DX11 GAUSSIAN BLUR /////////////////////////////

			inline float DX11FxGaussianBlur::GetSigma() const {

				return sigma_;

			}

		}

	}

}