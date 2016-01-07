/// \file dx11fx_lighting.h
/// \brief This file contains post process effects that affects lighting such as bloom, glow and color grading.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_lighting.h"

#include "tag.h"

#include "dx11fx_blur.h"
#include "dx11fx_filter.h"
#include "dx11fx_scaler.h"

#include "dx11/dx11gpgpu.h"
#include "dx11/dx11render_target.h"

namespace gi_lib {

	namespace dx11 {

		class DX11Material;
		class DX11Sampler;
		class DXStructuredbuffer;

		namespace fx {

			/// \brief This class is used to suppress color whose brightness falls under a given threshold.
			/// \author Raffaele D. Facendola
			class DX11FxBrightPass : public gi_lib::fx::FxBrightPass {

			public:

				/// \brief Create a new High-pass filter.
				/// \param offset Offset used to shift the exposure value of the scene. Higher values will cause more areas being left in darkness.
				DX11FxBrightPass(float offset);

				virtual void SetThreshold(float offset) override;

				virtual void SetKeyValue(float key_value) override;

				virtual void SetAverageLuminance(float average_luminance) override;

				virtual void Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;

			private:

				/// \brief Constant buffer used to pass the parameters to the filtering shader.
				struct Parameters {

					float gThreshold;										///< \brief Offset used to shift the exposure value of the scene.
					float gKeyValue;										///< \brief Target luminance of the scene.
					float gAverageLuminance;								///< \brief Current average luminance of the scene.

				};

				static const Tag kSourceTexture;							///< \brief Tag of the source texture to filter.

				static const Tag kSampler;									///< \brief Tag of the sampler used to sample the source texture.

				static const Tag kParameters;								///< \brief Tag of the constant buffer used to pass parameters to the shader.

				ObjectPtr<DX11Material> filter_shader_;						///< \brief Shader performing the scaling.

				ObjectPtr<DX11Sampler> sampler_;							///< \brief Sampler used to sample the source texture.

				ObjectPtr<DX11StructuredBuffer> parameters_;				///< \brief Parameters used to perform the filtering.

			};

			/// \brief Performs a bloom filtering of an image using DirectX11
			/// \author Raffaele D. Facendola
			class DX11FxBloom : public gi_lib::fx::FxBloom {

			public:

				/// \brief Create a new bloom filter.
				/// \param exposure_offset Exposure offset to filter out darker parts of the scene.
				/// \param bloom_strength How strong the bloom effect is. 
				/// \param sigma Sigma used to compute the Gaussian blur kernel.
				DX11FxBloom(float exposure_offset, float bloom_strength, float sigma);

				virtual void SetThreshold(float threshold) override;

				virtual float GetSigma() const override;

				virtual void SetSigma(float sigma) override;

				virtual void SetKeyValue(float key_value) override;

				virtual void SetAverageLuminance(float average_luminance) override;

				virtual void SetBloomStrength(float strength) override;

				virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;
								
			private:
				
				static const Tag kBase;									///< \brief Tag of the base texture to composite.

				static const Tag kBloom;								///< \brief Tag of the bloom texture to composite.
			
				static const Tag kBloomCompositeParameters;				///< \brief Tag of the constant buffer containing the parameters passed to the bloom composite shader.

				static const Tag kDownscaled;							///< \brief Tag of the downscaled texture to add.

				static const Tag kUpscaled;								///< \brief Tag of the upscaled texture to add.

				static const Tag kParameters;							///< \brief Tag of the constant buffer containing the shader parameters.

				static const Tag kSampler;								///< \brief Tag of the sampler used to sample the textures.
				
				static const size_t kDownscaledSurfaces;				///< \brief Number of downscaled surfaces.

				/// \brief Constant buffer used to pass the parameters to the filtering shader.
				struct BloomCompositeParameters {

					float gBloomStrength;								///< \brief Bloom strength.
					
				};
		
				/// \brief Initializes the working surfaces
				void InitializeSurfaces(const ObjectPtr<ITexture2D>& source);
				
				DX11FxGaussianBlur fx_blur_;									///< \brief Filter used to perform a Gaussian blur.

				DX11FxBrightPass fx_bright_pass_;								///< \brief Filter used to perform a Bright-pass process.

				DX11FxScaler fx_downscale_;										///< \brief Used to perform down scaling.
					
				ObjectPtr<DX11Material> upscale_shader_;						///< \brief Used to add blurred surfaces while upscaling.

				ObjectPtr<DX11Material> composite_shader_;						///< \brief Shader used to accumulate the various buffers.
				
				vector<ObjectPtr<DX11RenderTarget>> bright_surfaces_;			///< \brief Surface containing the "glowing" pixels. Only one surface.

				vector<ObjectPtr<DX11GPTexture2D>> blur_surfaces_;				///< \brief Surfaces containing the Gaussian blur result at half resolution per iteration.
				
				ObjectPtr<DX11Sampler> sampler_;								///< \brief Sampler used to sample the source texture.

				ObjectPtr<DX11StructuredBuffer> bloom_composite_parameters_;	///< \brief Parameters used to perform the filtering.

			};

			class DX11FxTonemap : public gi_lib::fx::FxTonemap {

			public:
				
				/// \brief Create a new tonemapping shader.
				DX11FxTonemap(float vignette, float key_value);

				virtual float GetVignette() const override;

				virtual void SetVignette(float vignette) override;

				virtual float GetKeyValue() const override;

				virtual void SetKeyValue(float key_value) override;

				virtual float GetAverageLuminance() const override;

				virtual void SetAverageLuminance(float average_luminance) override;
								
				virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) override;
				
			private:

				static const Tag kParameters;					///< \brief Tag of the constant buffer containing the tonemapping parameters

				static const Tag kSource;						///< \brief Tag of the unexposed buffer used as input of the tonemapping.

				static const Tag kDestination;					///< \brief Tag of the exposed buffer used as output of the tonemapping.
				
				/// \brief Parameters used by the shader
				struct Parameters {

					float vignette;					// Vignette factor.

					float key_value;				// Key value of the image. (sort of a 'mood')

					float average_luminance;		// Average linear luminance of the current frame.

					float reserved;				

				};

				Parameters parameters_;											///< \brief Shader parameters.

				bool dirty_;												///< \brief Whether the parameters are dirty and needs to be updated.

				ObjectPtr<DX11StructuredBuffer> tonemap_params_;			///< \brief Buffer containing the tonemap parameters.

				ObjectPtr<DX11Computation> tonemap_shader_;					///< \brief Shader performing the tonemapping stage.
				
			};

			//////////////////////////////////// DX11 FX BLOOM ////////////////////////////////////

			inline void DX11FxBloom::SetThreshold(float threshold){

				fx_bright_pass_.SetThreshold(threshold);

			}

			inline float DX11FxBloom::GetSigma() const{

				return fx_blur_.GetSigma();

			}

			inline void DX11FxBloom::SetSigma(float sigma){

				fx_blur_.SetSigma(sigma);

			}

			/////////////////////////////////// DX11 FX TONEMAPPING ////////////////////////////////////

			inline float DX11FxTonemap::GetVignette() const{
				
				return parameters_.vignette;

			}

			inline void DX11FxTonemap::SetVignette(float vignette){
				
				parameters_.vignette = vignette;

				dirty_ = true;

			}

			inline float DX11FxTonemap::GetKeyValue() const{
				
				return parameters_.key_value;

			}

			inline void DX11FxTonemap::SetKeyValue(float key_value){

				parameters_.key_value = key_value;

				dirty_ = true;

			}

			inline float DX11FxTonemap::GetAverageLuminance() const {

				return parameters_.average_luminance;

			}

			inline void DX11FxTonemap::SetAverageLuminance(float average_luminance) {

				parameters_.average_luminance = average_luminance;

				dirty_ = true;

			}
			
		}

	}

}