/// \file dx11gpgpu.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

#include "dx11.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

		/// \brief Represents a DirectX11 sampler state.
		/// \author Raffaele D. Facendola.
		class DX11Sampler : public IResource{

		public:

			/// \brief Structure used to create a sampler state from a plain description.
			struct FromDescription{

				USE_CACHE;

				/// \brief Texture mapping.
				TextureMapping texture_mapping;

				/// \brief Anisotropy level.
				unsigned int anisotropy_level;

				/// \brief Get the cache key associated to the structure.
				/// \return Returns the cache key associated to the structure.
				size_t GetCacheKey() const;

			};

			/// \brief Create a sampler state from a plain description.
			DX11Sampler(const FromDescription& description);

			virtual size_t GetSize() const override;

			/// \brief Get the sampler state.
			/// \return Returns the sampler state.
			ID3D11SamplerState& GetSamplerState() const;

		private:

			unique_com<ID3D11SamplerState> sampler_state_;

		};

	}

}

/////////////////////////////// DX11 SAMPLER ///////////////////////////////

inline ID3D11SamplerState& gi_lib::dx11::DX11Sampler::GetSamplerState() const{

	return *sampler_state_;

}

inline size_t gi_lib::dx11::DX11Sampler::GetSize() const
{

	return 0;

}

inline size_t gi_lib::dx11::DX11Sampler::FromDescription::GetCacheKey() const{

	// | ... | texture_mapping | anisotropy_level |
	//      40                 8                  0

	return (anisotropy_level & 0xFF) | (static_cast<unsigned int>(texture_mapping) << 8);

}