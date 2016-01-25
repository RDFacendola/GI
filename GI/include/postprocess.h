/// \file postprocess.h
/// \brief This file contains the classes used to perform the post processing phase of the rendering.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"
#include "graphics.h"
#include "timer.h"
#include "texture.h"
#include "fx\fx_image.h"
#include "fx\fx_postprocess.h"

using namespace gi_lib;

namespace gi {

	// Tonemapping

	const float kVignette = 0.5f;

	// Bloom

	const float kBloomExposure = 1.0f;
	const float kBloomStrength = 0.5f;
	const float kBloomBlurSigma = 1.67f;

	// Auto exposure

	const float kKeyValue = 0.4f;
	const float kMinLuminance = 0.0156f;
	const float kMaxLuminance = 64.0f;
	const float kLuminanceLowPercentage = 0.85f;
	const float kLuminanceHighPercentage = 0.95f;

	// Eye adaptation

	const float kMinAdaptLuminance = 0.2f;
	const float kMaxAdaptLuminance = 1.0f;
	const float kLuminanceAdaptationRate = 0.75f;

	class Postprocess {

	public:

		Postprocess(Resources& resources, Graphics& graphics);
		
		ObjectPtr<ITexture2D> Execute(ObjectPtr<ITexture2D> image, const Time& time);

	private:

		float UpdateLuminance(const ObjectPtr<ITexture2D>& image, const Time& time);

		float current_luminance_;										///< \brief Average luminance of the last processed image.

		Graphics& graphics_;

		ObjectPtr<fx::FxLuminance> fx_luminance_;						///<\ brief Used to calculate the luminance of the image.

		ObjectPtr<fx::FxBloom> fx_bloom_;								///< \brief Performs the bloom filter of the image.
		
		ObjectPtr<fx::FxTonemap> fx_tonemap_;							///< \brief Performs the tonemapping of the image.

		ObjectPtr<IGPTexture2D> output_;								///< \brief Contains the result of the post processing.
		
		ObjectPtr<IGPTexture2DCache> gp_texture_cache_;					///< \brief Cache of general-purposes textures.

		ObjectPtr<IRenderTargetCache> render_target_cache_;				///< \brief Cache of render-target textures.

	};

}