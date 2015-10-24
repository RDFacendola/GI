/// \file dx11fx_lighting.h
/// \brief This file contains post process effects that affects lighting such as bloom, glow and color grading.
///
/// \author Raffaele D. Facendola

#pragma once

#include "fx\fx_lighting.h"

#include "tag.h"

#include "dx11fx_blur.h"
#include "dx11fx_filter.h"

#include "dx11/dx11gpgpu.h"
#include "dx11/dx11render_target.h"

namespace gi_lib {

	namespace dx11 {

		namespace fx {

			/// \brief This class is used to suppress color whose brightness falls under a given threshold.
			/// \author Raffaele D. Facendola
			class DX11FxBrightPass : public gi_lib::fx::FxBrightPass {

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

			/// \brief Performs a bloom filtering of an image using DirectX11
			/// \author Raffaele D. Facendola
			class DX11FxBloom : public gi_lib::fx::FxBloom {

			public:

				/// \brief Create a new bloom filter.
				/// \param min_brightness Minimum brightness above of which the color is considered to be "glowing".
				/// \param sigma Sigma used to compute the Gaussian blur kernel.
				DX11FxBloom(float min_brightness, float sigma, const Vector2f& blur_scaling);

				virtual float GetMinBrightness() const override;

				virtual void SetMinBrightness(float min_brightness) override;

				virtual float GetSigma() const override;

				virtual void SetSigma(float sigma) override;

				virtual Vector2f GetBlurScaling() const override;

				virtual void SetBlurScaling(const Vector2f& scaling) override;

				virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) override;
								
			private:

				/// \brief Initializes the working surfaces
				void InitializeSurfaces(const ObjectPtr<ITexture2D>& source);

				static const Tag kSourceTexture;					///< \brief Tag of the source texture.

				static const Tag kGlowTexture;						///< \brief Tag of the blurred glow texture.

				static const Tag kSampler;							///< \brief Tag of the sampler used to sample the textures.

				Vector2f blur_scaling_;								///< \brief Scaling of the blurred surface. Set this value below 1 to increase blurring performances.

				DX11FxGaussianBlur fx_blur_;						///< \brief Filter used to perform a Gaussian blur.

				DX11FxBrightPass fx_high_pass_;						///< \brief Filter used to perform a High-pass process.
				
				ObjectPtr<DX11RenderTarget> glow_surface_;			///< \brief Surface containing the "glowing" pixels. Only one surface.

				ObjectPtr<DX11GPTexture2D> blur_surface_;			///< \brief Surface containing the Gaussian blur result.

				ObjectPtr<DX11Material> composite_shader_;			///< \brief Shader used to composite the blurred image with the source image.

				ObjectPtr<DX11Sampler> sampler_;					///< \brief Sampler used to sample the source texture.

			};

			class DX11FxTonemap : public gi_lib::fx::FxTonemap {

			public:
				
				/// \brief Create a new tonemapping shader.
				DX11FxTonemap(float vignette = 1.0f, float factor = 1.035f, float bias = 0.187f);

				virtual float GetVignette() const override;

				virtual void SetVignette(float vignette) override;

				virtual float GetFactor() const override;

				virtual void SetFactor(float factor) override;

				virtual float GetBias() const override;

				virtual void SetBias(float bias) override;

				virtual void Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) override;
				
			private:

				static const Tag kParameters;					///< \brief Tag of the constant buffer containing the tonemapping parameters

				static const Tag kSource;						///< \brief Tag of the unexposed buffer used as input of the tonemapping.

				static const Tag kDestination;					///< \brief Tag of the exposed buffer used as output of the tonemapping.
				
				/// \brief Parameters used by the shader
				struct Parameters {

					float vignette;					// Vignette factor.

					float factor;					// Multiplicative factor.

					float bias;						// Bias factor.

				};

				Parameters parameters_;											///< \brief Shader parameters.

				bool dirty_;												///< \brief Whether the parameters are dirty and needs to be updated.

				ObjectPtr<DX11StructuredBuffer> tonemap_params_;			///< \brief Buffer containing the tonemap parameters.

				ObjectPtr<DX11Computation> tonemap_shader_;					///< \brief Shader performing the tonemapping stage.
				
			};

			////////////////////////////////// DX11 FX BRIGHT PASS ////////////////////////////////

			inline float DX11FxBrightPass::GetThreshold() const {

				return threshold_;

			}

			//////////////////////////////////// DX11 FX BLOOM ////////////////////////////////////

			inline float DX11FxBloom::GetMinBrightness() const{

				return fx_high_pass_.GetThreshold();

			}

			inline void DX11FxBloom::SetMinBrightness(float min_brightness){

				fx_high_pass_.SetThreshold(min_brightness);

			}

			inline float DX11FxBloom::GetSigma() const{

				return fx_blur_.GetSigma();

			}

			inline void DX11FxBloom::SetSigma(float sigma){

				fx_blur_.SetSigma(sigma);

			}

			inline Vector2f DX11FxBloom::GetBlurScaling() const {

				return blur_scaling_;

			}

			inline void DX11FxBloom::SetBlurScaling(const Vector2f& scaling) {

				blur_scaling_ = scaling;

			}

			/////////////////////////////////// DX11 FX TONEMAPPING ////////////////////////////////////

			inline float DX11FxTonemap::GetVignette() const{
				
				return parameters_.vignette;

			}

			inline void DX11FxTonemap::SetVignette(float vignette){
				
				parameters_.vignette = vignette;

				dirty_ = true;

			}

			inline float DX11FxTonemap::GetFactor() const{
				
				return parameters_.factor;

			}

			inline void DX11FxTonemap::SetFactor(float factor){

				parameters_.factor = factor;

				dirty_ = true;

			}

			inline float DX11FxTonemap::GetBias() const{
				
				return parameters_.bias;

			}

			inline void DX11FxTonemap::SetBias(float bias){

				parameters_.bias = bias;

				dirty_ = true;

			}

		}

	}

}