/// \file dx11fx_postprocess.h
/// \brief This file contains post process effects that affects lighting such as bloom, glow and color grading.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_postprocess.h"

#include "tag.h"

#include "dx11fx_blur.h"
#include "dx11fx_filter.h"
#include "dx11fx_transform.h"

#include "dx11/dx11gpgpu.h"
#include "dx11/dx11render_target.h"

namespace gi_lib {

	namespace dx11 {

		class DX11Material;
		class DX11Sampler;
		class DXStructuredbuffer;

		/// \brief This class is used to suppress color whose brightness falls under a given threshold.
		/// \author Raffaele D. Facendola
		class DX11FxBrightPass : public fx::FxBrightPass {

		public:

			/// \brief Create a new High-pass filter.
			DX11FxBrightPass(const Parameters& parameters);

			virtual void SetThreshold(float offset) override;

			virtual void SetKeyValue(float key_value) override;

			virtual void SetAverageLuminance(float average_luminance) override;

			virtual void Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;

			virtual size_t GetSize() const override;

		private:

			/// \brief Constant buffer used to pass the parameters to the filtering shader.
			struct ShaderParameters {

				float gThreshold;										///< \brief Offset used to shift the exposure value of the scene.
				float gKeyValue;										///< \brief Target luminance of the scene.
				float gAverageLuminance;								///< \brief Current average luminance of the scene.

			};

			static const Tag kSourceTexture;							///< \brief Tag of the source texture to filter.

			static const Tag kSampler;									///< \brief Tag of the sampler used to sample the source texture.

			static const Tag kShaderParameters;							///< \brief Tag of the constant buffer used to pass parameters to the shader.

			ObjectPtr<DX11Material> filter_shader_;						///< \brief Shader performing the scaling.

			ObjectPtr<DX11Sampler> sampler_;							///< \brief Sampler used to sample the source texture.

			ObjectPtr<DX11StructuredBuffer> shader_parameters_;			///< \brief Parameters used to perform the filtering.

		};

		/// \brief Performs a bloom filtering of an image using DirectX11
		/// \author Raffaele D. Facendola
		class DX11FxBloom : public fx::FxBloom {

		public:

			/// \brief Create a new bloom filter.
			DX11FxBloom(const Parameters& parameters);

			virtual void SetThreshold(float threshold) override;

			virtual float GetSigma() const override;

			virtual void SetSigma(float sigma) override;

			virtual void SetKeyValue(float key_value) override;

			virtual void SetAverageLuminance(float average_luminance) override;

			virtual void SetBloomStrength(float strength) override;

			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;
								
			virtual size_t GetSize() const override;
				
		private:
				
			static const Tag kBase;									///< \brief Tag of the base texture to composite.

			static const Tag kBloom;								///< \brief Tag of the bloom texture to composite.
			
			static const Tag kShaderParameters;						///< \brief Tag of the constant buffer containing the parameters;

			static const Tag kDownscaled;							///< \brief Tag of the downscaled texture to add.

			static const Tag kUpscaled;								///< \brief Tag of the upscaled texture to add.

			static const Tag kSampler;								///< \brief Tag of the sampler used to sample the textures.
				
			static const size_t kDownscaledSurfaces;				///< \brief Number of downscaled surfaces.

			/// \brief Constant buffer used to pass the parameters to the filtering shader.
			struct ShaderParameters {

				float gBloomStrength;								///< \brief Bloom strength.
					
			};
		
			/// \brief Initializes the working surfaces
			void InitializeSurfaces(const ObjectPtr<ITexture2D>& source);

			DX11FxGaussianBlur fx_blur_;									///< \brief Filter used to perform a Gaussian blur.

			DX11FxBrightPass fx_bright_pass_;								///< \brief Filter used to perform a Bright-pass process.

			DX11FxScale fx_downscale_;										///< \brief Used to perform down scaling.
					
			ObjectPtr<DX11Material> upscale_shader_;						///< \brief Used to add blurred surfaces while upscaling.

			ObjectPtr<DX11Material> composite_shader_;						///< \brief Shader used to accumulate the various buffers.
				
			vector<ObjectPtr<DX11RenderTarget>> bright_surfaces_;			///< \brief Surface containing the "glowing" pixels. Only one surface.

			vector<ObjectPtr<DX11GPTexture2D>> blur_surfaces_;				///< \brief Surfaces containing the Gaussian blur result at half resolution per iteration.
				
			ObjectPtr<DX11Sampler> sampler_;								///< \brief Sampler used to sample the source texture.

			ObjectPtr<DX11StructuredBuffer> shader_parameters_;				///< \brief Parameters used to perform the filtering.

		};

		/// \brief Performs a tonemapping of an image using DirectX11.
		/// \author Raffaele D. Facendola
		class DX11FxTonemap : public fx::FxTonemap {

		public:
				
			/// \brief Create a new tonemapping shader.
			DX11FxTonemap(const Parameters& parameters);
				
			virtual void SetVignette(float vignette) override;
				
			virtual void SetKeyValue(float key_value) override;
				
			virtual void SetAverageLuminance(float average_luminance) override;
								
			virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) override;

			virtual size_t GetSize() const override;


		private:

			static const Tag kShaderParameters;				///< \brief Tag of the constant buffer containing the tonemapping parameters

			static const Tag kSource;						///< \brief Tag of the unexposed buffer used as input of the tonemapping.

			static const Tag kDestination;					///< \brief Tag of the exposed buffer used as output of the tonemapping.
				
			/// \brief Parameters used by the shader
			struct ShaderParameters {

				float gVignette;					// Vignette factor.

				float gKeyValue;					// Key value of the image. (sort of a 'mood')

				float gAverageLuminance;			// Average linear luminance of the current frame.

				float reserved;				

			};

			ObjectPtr<DX11StructuredBuffer> shader_parameters_;			///< \brief Buffer containing the tonemap parameters.

			ObjectPtr<DX11Computation> tonemap_shader_;					///< \brief Shader performing the tonemapping stage.
				
		};
		
		////////////////////////////////// DX11 FX BRIGHT PASS ////////////////////////////////

		INSTANTIABLE(fx::FxBrightPass, DX11FxBrightPass, fx::FxBrightPass::Parameters);

		inline size_t DX11FxBrightPass::GetSize() const {

			return 0;

		}
			
		//////////////////////////////////// DX11 FX BLOOM ////////////////////////////////////

		INSTANTIABLE(fx::FxBloom, DX11FxBloom, fx::FxBloom::Parameters);

		inline void DX11FxBloom::SetThreshold(float threshold){

			fx_bright_pass_.SetThreshold(threshold);

		}

		inline float DX11FxBloom::GetSigma() const{

			return fx_blur_.GetSigma();

		}

		inline void DX11FxBloom::SetSigma(float sigma){

			fx_blur_.SetSigma(sigma);

		}

		inline size_t DX11FxBloom::GetSize() const {

			return 0;

		}

		/////////////////////////////////// DX11 FX TONEMAPPING ////////////////////////////////////

		INSTANTIABLE(fx::FxTonemap, DX11FxTonemap, fx::FxTonemap::Parameters);

		inline size_t DX11FxTonemap::GetSize() const {

			return 0;

		}
		
		/////////////////////////////////////// RESOURCE CAST ///////////////////////////////////////

		inline ObjectPtr<DX11FxBrightPass> resource_cast(const ObjectPtr<fx::FxBrightPass>& resource) {

			return ObjectPtr<DX11FxBrightPass>(resource.Get());

		}

		inline ObjectPtr<DX11FxBloom> resource_cast(const ObjectPtr<fx::FxBloom>& resource) {

			return ObjectPtr<DX11FxBloom>(resource.Get());

		}

		inline ObjectPtr<DX11FxTonemap> resource_cast(const ObjectPtr<fx::FxTonemap>& resource) {

			return ObjectPtr<DX11FxTonemap>(resource.Get());

		}

	}

}