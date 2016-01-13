#include "dx11/fx/dx11fx_lighting.h"

#include "core.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11render_target.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

////////////////////////////////// DX11 FX BRIGHT PASS ////////////////////////////////

const Tag DX11FxBrightPass::kSourceTexture = "gSource";

const Tag DX11FxBrightPass::kSampler = "gSourceSampler";

const Tag DX11FxBrightPass::kShaderParameters = "Parameters";

DX11FxBrightPass::DX11FxBrightPass(const Parameters& parameters) {

	filter_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\bright_pass.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

	shader_parameters_ = new DX11StructuredBuffer(sizeof(ShaderParameters));

	//One-time setup

	filter_shader_->SetInput(kSampler,
							 ObjectPtr<ISampler>(sampler_));

	filter_shader_->SetInput(kShaderParameters,
							 ObjectPtr<IStructuredBuffer>(shader_parameters_));

	SetThreshold(parameters.threshold_);
	SetKeyValue(parameters.key_value_);
	SetAverageLuminance(parameters.average_luminance_);

}

void DX11FxBrightPass::SetThreshold(float threshold) {

	shader_parameters_->Lock<ShaderParameters>()->gThreshold = threshold;
	
	shader_parameters_->Unlock();

}

void DX11FxBrightPass::SetKeyValue(float key_value){

	shader_parameters_->Lock<ShaderParameters>()->gKeyValue = key_value;

	shader_parameters_->Unlock();

}

void DX11FxBrightPass::SetAverageLuminance(float average_luminance){

	shader_parameters_->Lock<ShaderParameters>()->gAverageLuminance = average_luminance;

	shader_parameters_->Unlock();

}

void DX11FxBrightPass::Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) {

	auto device_context = DX11Graphics::GetInstance().GetImmediateContext();
 
 	auto dx_destination = ::resource_cast(destination);
 
 	// Shader setup
 
 	dx_destination->ClearDepth(*device_context);				// Clear the existing depth
 
	filter_shader_->SetInput(kSourceTexture, 
							 source);							// Source texture
 
 	dx_destination->Bind(*device_context);						// Destination texture
 
	filter_shader_->Bind(*device_context);						// Bind the shader
 		
 	// Render a quad
 
 	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
 	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 	
 	device_context->Draw(6, 0);									// Fire the rendering
 
 	// Unbind the material from the context
 
	filter_shader_->Unbind(*device_context);

 	dx_destination->Unbind(*device_context);

}

//////////////////////////////////// DX11 FX BLOOM ////////////////////////////////////

const Tag DX11FxBloom::kBase = "gBase";
const Tag DX11FxBloom::kBloom = "gBloom";
const Tag DX11FxBloom::kDownscaled = "gDownscaled";
const Tag DX11FxBloom::kUpscaled = "gUpscaled";
const Tag DX11FxBloom::kSampler = "gSampler";
const Tag DX11FxBloom::kShaderParameters = "Parameters";

const size_t DX11FxBloom::kDownscaledSurfaces = 6;

DX11FxBloom::DX11FxBloom(const Parameters& parameters) :
	fx_downscale_(gi_lib::fx::FxScaler::Parameters{}),
	fx_blur_(gi_lib::fx::FxGaussianBlur::Parameters{ parameters.sigma_}),
	fx_bright_pass_(gi_lib::fx::FxBrightPass::Parameters{ parameters.threshold_, parameters.key_value_, parameters.average_luminance_}) {
	
	composite_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\bloom_composite.hlsl" });

	upscale_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\bloom_upscale.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

	shader_parameters_ = new DX11StructuredBuffer(sizeof(ShaderParameters));
	
	//One-time setup

	composite_shader_->SetInput(kSampler,
								ObjectPtr<ISampler>(sampler_));

	composite_shader_->SetInput(kShaderParameters,
								ObjectPtr<IStructuredBuffer>(shader_parameters_));

	upscale_shader_->SetInput(kSampler,
							  ObjectPtr<ISampler>(sampler_));

	SetBloomStrength(parameters.strength_);

}

void DX11FxBloom::Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) {

 	// Lazy initialization of the surfaces
 	InitializeSurfaces(source);

	// Performs a bright pass of the source texture (the destination is also downscaled here)

	fx_bright_pass_.Filter(source,
						   ObjectPtr<IRenderTarget>(bright_surfaces_[0]));

	// Downscale recursively

	for (size_t index = 1; index < bright_surfaces_.size(); ++index) {

		fx_downscale_.Copy((*bright_surfaces_[index - 1])[0],
						   ObjectPtr<IRenderTarget>(bright_surfaces_[index]));

	}

	// Blur each downscaled surface

	for (size_t index = 0; index < bright_surfaces_.size(); ++index) {

		fx_blur_.Blur((*bright_surfaces_[index])[0],
					  ObjectPtr<IGPTexture2D>(blur_surfaces_[index]));
						
		// Additional blur passes to smooth out the jagginess of lower-resolution surfaces.

		for (size_t passes = 0; passes < index; ++passes) {

			fx_blur_.Blur(blur_surfaces_[index]->GetTexture(),
						  ObjectPtr<IGPTexture2D>(blur_surfaces_[index]));
		
		}

	}

	// We only draw GPU-generated quads
			
	auto device_context = DX11Graphics::GetInstance().GetImmediateContext();

	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Upscaling & Bloom add - Recycle the bright surfaces as destination of the upscaling

	ObjectPtr<ITexture2D> downscaled = blur_surfaces_.back()->GetTexture();

	for (size_t index = bright_surfaces_.size() - 1; index > 0; --index) {

		upscale_shader_->SetInput(kDownscaled,
								  downscaled);

		upscale_shader_->SetInput(kUpscaled,
								  blur_surfaces_[index - 1]->GetTexture());

		bright_surfaces_[index - 1]->ClearDepth(*device_context);

		bright_surfaces_[index - 1]->Bind(*device_context);		// Output

		upscale_shader_->Bind(*device_context);
				
		// Render a quad

		device_context->Draw(6, 0);								// Fire the rendering

	}
		
	upscale_shader_->Unbind(*device_context);

	// Bloom compositing

	auto dx_destination = ::resource_cast(destination);

	dx_destination->ClearDepth(*device_context);												// Clear the existing depth

	composite_shader_->SetInput(kBase,
								source);														// Source texture
	
	composite_shader_->SetInput(kBloom,
								(*bright_surfaces_[0])[0]);

	dx_destination->Bind(*device_context);														// Destination texture

	composite_shader_->Bind(*device_context);													// Bind the shader

	// Render a quad
	
	device_context->Draw(6, 0);									// Fire the rendering

	// Cleanup

	composite_shader_->Unbind(*device_context);

	dx_destination->Unbind(*device_context);
	
}

void DX11FxBloom::InitializeSurfaces(const ObjectPtr<ITexture2D>& source) {

	// Blur and glow surfaces have the same size and format, it is sufficient to check only one of those.

	auto format = source->GetFormat();
	auto width = source->GetWidth();
	auto height = source->GetHeight();

	if (bright_surfaces_.size() == 0 ||
		bright_surfaces_[0]->GetWidth() != (width >> 1) ||
		bright_surfaces_[0]->GetHeight() != (height >> 1) ||
		format != source->GetFormat()) {

		// Release the temporary resources

		for (auto&& surface : blur_surfaces_) {

			DX11GPTexture2D::PushToCache(surface);

		}

		for (auto&& surface : bright_surfaces_) {

			DX11RenderTarget::PushToCache(surface);

		}

		blur_surfaces_.clear();
		bright_surfaces_.clear();

		// Build the downscaled versions

		for (size_t index = 1; index < kDownscaledSurfaces && width >> index > 0 && height >> index > 0; ++index) {

			bright_surfaces_.push_back(DX11RenderTarget::PopFromCache(width >> index,
																	  height >> index,
																	  { format },
																	  false,
																	  true));

			blur_surfaces_.push_back(DX11GPTexture2D::PopFromCache(width >> index, 
																   height >> index, 
																   format, 
																   true));

		}

	}
	
}

void DX11FxBloom::SetKeyValue(float key_value) {

	fx_bright_pass_.SetKeyValue(key_value);

}

void DX11FxBloom::SetAverageLuminance(float average_luminance) {

	fx_bright_pass_.SetAverageLuminance(average_luminance);

}

void DX11FxBloom::SetBloomStrength(float strength) {

	// Since each downscaled version of the bloom adds color, we just scale down the strength accordingly (such that it is not anymore a sum but an average)

	shader_parameters_->Lock<ShaderParameters>()->gBloomStrength= strength * 1.0f / kDownscaledSurfaces;

	shader_parameters_->Unlock();

}

/////////////////////////////////// DX11 FX TONEMAPPING ////////////////////////////////////

const Tag DX11FxTonemap::kShaderParameters = "TonemapParams";
const Tag DX11FxTonemap::kSource = "gUnexposed";
const Tag DX11FxTonemap::kDestination = "gExposed";

DX11FxTonemap::DX11FxTonemap(const Parameters& parameters) {

	// Tonemap setup

	tonemap_shader_ = DX11Resources::GetInstance().Load<IComputation, IComputation::CompileFromFile>({ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\tonemap.hlsl" });

	shader_parameters_ = new DX11StructuredBuffer(sizeof(ShaderParameters));

	// One-time setup

	tonemap_shader_->SetInput(kShaderParameters,
							  ObjectPtr<IStructuredBuffer>(shader_parameters_));
	
	SetVignette(parameters.vignette_);
	SetKeyValue(parameters.key_value_);
	SetAverageLuminance(parameters.average_luminance_);

}

void DX11FxTonemap::SetVignette(float vignette) {

	shader_parameters_->Lock<ShaderParameters>()->gVignette = vignette;

	shader_parameters_->Unlock();
	
}

void DX11FxTonemap::SetKeyValue(float key_value) {

	shader_parameters_->Lock<ShaderParameters>()->gKeyValue = key_value;

	shader_parameters_->Unlock();

}

void DX11FxTonemap::SetAverageLuminance(float average_luminance) {

	shader_parameters_->Lock<ShaderParameters>()->gAverageLuminance = average_luminance;

	shader_parameters_->Unlock();

}

void DX11FxTonemap::Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination){
	
	// Shader setup

	tonemap_shader_->SetInput(kSource,
							  source);

	tonemap_shader_->SetOutput(kDestination,
							   destination);			
	
	// Dispatch one thread for each source's pixel

	tonemap_shader_->Dispatch(*DX11Graphics::GetInstance().GetImmediateContext(),			
							  source->GetWidth(),
							  source->GetHeight(),
							  1);

}