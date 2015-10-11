#include "dx11/fx/dx11fx_filter.h"

#include "core.h"

#include "dx11/dx11graphics.h"

using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::dx11::fx;

////////////////////////////////// DX11 FX HIGH PASS ////////////////////////////////

const Tag DX11FxHighPass::kSourceTexture = "gSource";

const Tag DX11FxHighPass::kSampler = "gSourceSampler";

const Tag DX11FxHighPass::kParameters = "Parameters";

DX11FxHighPass::DX11FxHighPass(float threshold) {

	filter_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\high_pass.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, 16 });

	parameters_ = new DX11StructuredBuffer(sizeof(Parameters));

	//One-time setup

	filter_shader_->SetInput(kSampler,
							 ObjectPtr<ISampler>(sampler_));

	filter_shader_->SetInput(kParameters,
							 ObjectPtr<IStructuredBuffer>(parameters_));

	SetThreshold(threshold);

}

void DX11FxHighPass::SetThreshold(float threshold) {

	threshold_ = threshold;

	auto parameters = reinterpret_cast<Parameters*>(parameters_->Lock());

	parameters->gThreshold = threshold;

	parameters_->Unlock();

}

void DX11FxHighPass::Filter(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) {

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