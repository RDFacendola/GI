#include "dx11/fx/dx11fx_lighting.h"

#include "core.h"

#include "dx11/dx11graphics.h"

using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::dx11::fx;


////////////////////////////////// DX11 FX HIGH PASS ////////////////////////////////

const Tag DX11FxBrightPass::kSourceTexture = "gSource";

const Tag DX11FxBrightPass::kSampler = "gSourceSampler";

const Tag DX11FxBrightPass::kParameters = "Parameters";

DX11FxBrightPass::DX11FxBrightPass(float threshold) {

	filter_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\bright_pass.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

	parameters_ = new DX11StructuredBuffer(sizeof(Parameters));

	//One-time setup

	filter_shader_->SetInput(kSampler,
							 ObjectPtr<ISampler>(sampler_));

	filter_shader_->SetInput(kParameters,
							 ObjectPtr<IStructuredBuffer>(parameters_));

	SetThreshold(threshold);

}

void DX11FxBrightPass::SetThreshold(float threshold) {

	threshold_ = threshold;

	auto parameters = reinterpret_cast<Parameters*>(parameters_->Lock());

	parameters->gThreshold = threshold;

	parameters_->Unlock();

}

void DX11FxBrightPass::Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) {

	auto device_context = DX11Graphics::GetInstance().GetImmediateContext();
 
 	auto dx_destination = resource_cast(destination);
 
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

const Tag DX11FxBloom::kSourceTexture = "gSource";

const Tag DX11FxBloom::kGlowTexture = "gGlow";

const Tag DX11FxBloom::kSampler = "gSourceSampler";

DX11FxBloom::DX11FxBloom(float min_brightness, float sigma, const Vector2f& blur_scaling) :
	fx_blur_(sigma),
	fx_high_pass_(min_brightness),
	blur_scaling_(blur_scaling){

	composite_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\bloom_composite.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

	//One-time setup

	composite_shader_->SetInput(kSampler,
								ObjectPtr<ISampler>(sampler_));

}

void DX11FxBloom::Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) {

 	// Lazy initialization of the blur surface and glow surface
 	InitializeSurfaces(source);

 	
 	// High pass filtering - Source => Glow surface
 
 	fx_high_pass_.Filter(source, 
 						 ObjectPtr<IRenderTarget>(glow_surface_));
 
 	// Gaussian blur - Glow surface => Blur surface
 
 	fx_blur_.Blur((*glow_surface_)[0], 
 				  ObjectPtr<IGPTexture2D>(blur_surface_));

	// Bloom composite - Blur surface + Source => Destination

	auto device_context = DX11Graphics::GetInstance().GetImmediateContext();

	auto dx_destination = resource_cast(destination);

	dx_destination->ClearDepth(*device_context);												// Clear the existing depth

	composite_shader_->SetInput(kSourceTexture,
								source);														// Source texture
	
	composite_shader_->SetInput(kGlowTexture,
								ObjectPtr<ITexture2D>(blur_surface_->GetTexture()));			// Glow texture

	dx_destination->Bind(*device_context);														// Destination texture

	composite_shader_->Bind(*device_context);													// Bind the shader

	// Render a quad

	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	device_context->Draw(6, 0);									// Fire the rendering

	// Cleanup

	composite_shader_->Unbind(*device_context);

	dx_destination->Unbind(*device_context);


}

void DX11FxBloom::InitializeSurfaces(const ObjectPtr<ITexture2D>& source) {

	// Blur and glow surfaces have the same size and format, it is sufficient to check only one of those.

	auto format = resource_cast(source)->GetFormat();

	unsigned int scaled_width = static_cast<unsigned int>(source->GetWidth() * blur_scaling_(0));
	unsigned int scaled_height = static_cast<unsigned int>(source->GetHeight() * blur_scaling_(1));

	if (blur_surface_ == nullptr ||
		blur_surface_->GetWidth() != scaled_width ||
		blur_surface_->GetHeight() != scaled_height ||
		format != blur_surface_->GetFormat()) {

		blur_surface_ = new DX11GPTexture2D(scaled_width,
											scaled_height,
											format);
		
		glow_surface_= new DX11RenderTarget(scaled_width,
											scaled_height,
											{ format });
	}
	
}

/////////////////////////////////// DX11 FX TONEMAPPING ////////////////////////////////////

const Tag DX11FxTonemap::kParameters = "TonemapParams";
const Tag DX11FxTonemap::kSource = "gUnexposed";
const Tag DX11FxTonemap::kDestination = "gExposed";

DX11FxTonemap::DX11FxTonemap(float vignette, float factor, float bias) {

	SetVignette(vignette);
	SetFactor(factor);
	SetBias(bias);

	// Tonemap setup

	tonemap_shader_ = DX11Resources::GetInstance().Load<IComputation, IComputation::CompileFromFile>({ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\tonemap.hlsl" });

	tonemap_params_ = new DX11StructuredBuffer(sizeof(Parameters));

	// One-time setup

	tonemap_shader_->SetInput(kParameters,
							  ObjectPtr<IStructuredBuffer>(tonemap_params_));

}

void DX11FxTonemap::Process(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination){
	
	// Update shader parameters

	if (dirty_) {
		
		memcpy_s(tonemap_params_->Lock(),
				 tonemap_params_->GetSize(),
				 &parameters_,
				 sizeof(parameters_));

		tonemap_params_->Unlock();

		dirty_ = false;

	}

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
