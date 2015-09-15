/// \file dx11gpgpu.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include "sampler.h"
#include "debug.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"
#include "instance_builder.h"

namespace gi_lib{

	namespace dx11{

		using windows::COMPtr;

		/// \brief Represents a DirectX11 sampler state.
		/// \author Raffaele D. Facendola.
		class DX11Sampler : public ISampler{

		public:

			/// \brief Create a sampler state from a plain description.
			DX11Sampler(const FromDescription& description);

			virtual size_t GetSize() const override;

			virtual unsigned int GetMaxAnisotropy() const override;

			virtual TextureMapping GetTextureMapping() const override;

			/// \brief Get the sampler state view.
			/// \return Returns the sampler state view.
			SamplerStateView GetSamplerStateView();

		private:

			COMPtr<ID3D11SamplerState> sampler_state_;

			unsigned int max_anisotropy_;					///< \brief Maximum anisotropy.

			TextureMapping texture_mapping_;				///< \brief Texture mapping along each dimension.

		};

		/// \brief Downcasts an ITexture2D to the proper concrete type.
		ObjectPtr<DX11Sampler> resource_cast(const ObjectPtr<ISampler>& resource);

		/////////////////////////////// DX11 SAMPLER ///////////////////////////////
		
		INSTANTIABLE(ISampler, DX11Sampler, DX11Sampler::FromDescription);

		inline SamplerStateView DX11Sampler::GetSamplerStateView(){

			return SamplerStateView(this,
							   sampler_state_);

		}

		inline size_t DX11Sampler::GetSize() const
		{

			return 0;

		}

		inline unsigned int DX11Sampler::GetMaxAnisotropy() const{

			return max_anisotropy_;

		}

		inline TextureMapping DX11Sampler::GetTextureMapping() const{

			return texture_mapping_;

		}
	
		/////////////////////////////// RESOURCE CAST /////////////////////////////////

		inline ObjectPtr<DX11Sampler> resource_cast(const ObjectPtr<ISampler>& resource){

			return ObjectPtr<DX11Sampler>(resource.Get());

		}


	}

}

