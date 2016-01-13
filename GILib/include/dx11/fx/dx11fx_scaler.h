/// \file dx11fx_scaler.h
/// \brief This file contains classes used to scale a texture using the GPU.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_scaler.h"

#include "..\dx11material.h"
#include "..\dx11sampler.h"

#include "..\..\tag.h"

namespace gi_lib {

	namespace dx11 {

		/// \brief Shader used to scale a source texture onto a render target.
		class DX11FxScaler : public fx::FxScaler{

		public:

			DX11FxScaler(const Parameters& parameters);

			virtual void Copy(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;

			virtual size_t GetSize() const override;


		private:

			// Tonemapping

			static const Tag kSourceTexture;							///< \brief Tag of the source texture to scale.

			static const Tag kSampler;									///< \brief Tag of sampler used to sample the source texture.

 			ObjectPtr<DX11Material> scaling_shader_;					///< \brief Shader performing the scaling.
 
 			ObjectPtr<DX11Sampler> sampler_;							///< \brief Sampler used to sample the source texture.
 				
		
		};

		/////////////////////////// DX11 FX SCALER ///////////////////////////////////

		INSTANTIABLE(fx::FxScaler, DX11FxScaler, fx::FxScaler::Parameters);

		/////////////////////////////////////// RESOURCE CAST ///////////////////////////////////////

		inline ObjectPtr<DX11FxScaler> resource_cast(const ObjectPtr<fx::FxScaler>& resource) {

			return ObjectPtr<DX11FxScaler>(resource.Get());

		}
		
	}

}