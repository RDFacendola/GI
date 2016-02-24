/// \file dx11fx_filter.h
/// \brief This file contains classes used to filter a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>

#include "fx\fx_filter.h"

#include "..\dx11gpgpu.h"

#include "..\..\tag.h"

namespace gi_lib {

	namespace dx11 {

		/// \brief Gaussian filter.
		class DX11FxGaussianBlur: public fx::FxGaussianBlur {

		public:

			/// \brief Create a new Gaussian blur filter.
			DX11FxGaussianBlur(const Parameters& parameters);
				
			virtual float GetSigma() const override;

			virtual void SetSigma(float sigma) override;

			virtual void Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination, const Vector2i& offset) override;
			
			virtual size_t GetSize() const override;

		private:

			// Tonemapping

			static const Tag kSourceTexture;							///< \brief Tag of the source texture to blur.

			static const Tag kDestinationTexture;						///< \brief Tag of the destination texture to write.

			ObjectPtr<DX11StructuredArray> kernel_;						///< \brief Contains the actual values of the kernel.

			ObjectPtr<DX11StructuredBuffer> parameters_;				///< \brief Contains the parameters for the gaussian blur.

			ObjectPtr<DX11Computation> hblur_shader_;					///< \brief Used to perform a horizontal blur pass.

			ObjectPtr<DX11Computation> vblur_shader_;					///< \brief Used to perform a vertical blur pass.

			std::unique_ptr<IGPTexture2DCache> gp_cache_;				///< \brief Pointer to the general-purpose texture cache.

			float sigma_;												///< \brief Variance of the Gaussian function.

			int radius_;												///< \brief Kernel radius.

		};
			
		///////////////////////////// DX11 GAUSSIAN BLUR /////////////////////////////

		INSTANTIABLE(fx::FxGaussianBlur, DX11FxGaussianBlur, fx::FxGaussianBlur::Parameters);

		inline float DX11FxGaussianBlur::GetSigma() const {

			return sigma_;

		}
			
		inline size_t DX11FxGaussianBlur::GetSize() const {

			return 0;

		}

		///////////////////////////// RESOURCE CAST ////////////////////////////////

		inline ObjectPtr<DX11FxGaussianBlur> resource_cast(const ObjectPtr<fx::FxGaussianBlur>& resource) {

			return ObjectPtr<DX11FxGaussianBlur>(resource.Get());

		}

	}

}