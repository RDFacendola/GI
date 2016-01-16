#include "postprocess.h"

#include "fx/fx_postprocess.h"
#include "fx/fx_image.h"

using namespace gi;
using namespace gi_lib;
using namespace gi_lib::fx;

#include "dx11/dx11texture.h"
#include "dx11/dx11render_target.h"

using namespace gi_lib::dx11;

//////////////////////////////// POSTPROCESS ///////////////////////////////

Postprocess::Postprocess(Resources& resources) {

	fx_luminance_ = resources.Load<FxLuminance, FxLuminance::Parameters>({ kMinLuminance, kMaxLuminance, kLuminanceLowPercentage, kLuminanceHighPercentage });

	fx_bloom_ = resources.Load<FxBloom, FxBloom::Parameters>({ kBloomExposure, kBloomBlurSigma, kKeyValue, 0.f, kBloomStrength });

	fx_tonemap_ = resources.Load<FxTonemap, FxTonemap::Parameters>({ kVignette, kKeyValue, 0.f });

	gp_texture_cache_ = resources.Load<IGPTexture2DCache, IGPTexture2DCache::Singleton>({});

	render_target_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	current_luminance_ = 0.f;

}

ObjectPtr<ITexture2D> Postprocess::Execute(ObjectPtr<gi_lib::ITexture2D> image, const Time& time) {

	// Initialization of the working surfaces

	if (output_) {

		// Discard the previous image.
		gp_texture_cache_->PushToCache(output_);

	}

	output_ = gp_texture_cache_->PopFromCache(image->GetWidth(),
											  image->GetHeight(),
											  TextureFormat::RGBA_HALF_UNORM);

	auto bloom_output = render_target_cache_->PopFromCache(image->GetWidth(),
														   image->GetHeight(),
														   { TextureFormat::RGB_FLOAT },
														   false);
	
	// Image >> Bloom >> Tonemap

	UpdateLuminance(image, time);

	fx_bloom_->SetAverageLuminance(current_luminance_);
	
	fx_tonemap_->SetAverageLuminance(current_luminance_);

	// Bloom

	fx_bloom_->Process(image,
					   bloom_output);

	// Tonemap
	
	fx_tonemap_->Process((*bloom_output)[0],
						 output_);

	// Done

	render_target_cache_->PushToCache(bloom_output);

	return output_->GetTexture();

}

float Postprocess::UpdateLuminance(const ObjectPtr<gi_lib::ITexture2D>& image, const Time& time) {

	// Eye adaptation: smoothly interpolate between the last luminance and the current one.

	auto last_luminance = current_luminance_;

	current_luminance_ = fx_luminance_->ComputeAverageLuminance(image);

	current_luminance_ = std::fmaxf(kMinAdaptLuminance, std::fminf(kMaxAdaptLuminance, current_luminance_));

	current_luminance_ = last_luminance + (current_luminance_ - last_luminance) * (1.f - std::expf(-time.GetDeltaSeconds() * kLuminanceAdaptationRate));

	return current_luminance_;

}
