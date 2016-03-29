/// \file dx11fx_image.h
/// \brief This file contains classes used to get image infos and histograms using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_image.h"

#include "..\dx11gpgpu.h"

#include "instance_builder.h"
#include "dx11fx_transform.h"

namespace gi_lib {

	namespace dx11 {
		
		/// \brief Shader used to compute the average luminance of an image.
		/// \author Raffaele D. Facendola
		class DX11FxLuminance : public gi_lib::fx::FxLuminance {

		public:

			DX11FxLuminance(const Parameters& parameters);

			virtual float ComputeAverageLuminance(const ObjectPtr<ITexture2D>& source) const override;

			virtual void SetMinLuminance(float min_luminance) override;

			virtual void SetMaxLuminance(float max_luminance) override;

			virtual void SetLowPercentage(float low_percentage) override;

			virtual void SetHighPercentage(float high_percentage) override;

			virtual size_t GetSize() const override;

		private:

			struct ShaderParameters{

				float gLogMinimum;							///< \brief Minimum log luminance.
				float gLogMaximum;							///< \brief Maximum log luminance.

				Vector2f reserved;

			};

			static const unsigned int kBinCount;								///< \brief Bins used for the histogram.

			static const Tag kSourceTexture;									///< \brief Tag of the source texture to blur.

			static const Tag kHistogram;										///< \brief Tag of the histogram.

			static const Tag kParameters;										///< \brief Tag of the shader parameters.

            unsigned int downscale_;                                            ///< \brief Number of time the source image is downscaled before computing its average luminance.

			float low_percentage_;												///< \brief Percentage of samples whose luminance is below the calculated average.
						
			float high_percentage_;												///< \brief Percentage of samples whose luminance is above the calculated average.

			float min_log_luminance_;

			float max_log_luminance_;

            DX11FxScale fx_downscale_;								    		///< \brief Used to perform down scaling.

            std::unique_ptr<IRenderTargetCache> rt_cache_;					///< \brief Pointer to the render-target texture cache.
            
			ObjectPtr<DX11Computation> clear_shader_;							///< \brief Used to clear the histogram.

			ObjectPtr<DX11Computation> luminance_shader_;						///< \brief Used to calculate the luminance of the image.

			ObjectPtr<DX11ScratchStructuredArray> log_luminance_histogram_;		///< \brief Contains the histogram of the log luminance.

			ObjectPtr<DX11StructuredBuffer> luminance_parameters_;				///< \brief Parameters passed to the compute shader.

		};

		/////////////////////////////////////// DX11 FX LUMINANCE /////////////////////////////////////// 

		INSTANTIABLE(fx::FxLuminance, DX11FxLuminance, fx::FxLuminance::Parameters);

		/////////////////////////////////////// RESOURCE CAST ///////////////////////////////////////

		inline ObjectPtr<DX11FxLuminance> resource_cast(const ObjectPtr<fx::FxLuminance>& resource) {

			return ObjectPtr<DX11FxLuminance>(resource.Get());

		}
			
	}

}