#include "dx11/fx/dx11fx_transform.h"

#include "core.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11render_target.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

/////////////////////////// DX11 FX SCALER ///////////////////////////////////

const Tag DX11FxScale::kSourceTexture = "gSource";

const Tag DX11FxScale::kSampler = "gSampler";

DX11FxScale::DX11FxScale(const Parameters&){
		
	scaling_shader_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\common\\passthrough_ps.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

	//Sampler is set only once

	scaling_shader_->SetInput(kSampler,
							  ObjectPtr<ISampler>(sampler_));

}

void DX11FxScale::Copy(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IRenderTarget>& destination) const {

 	auto device_context = DX11Graphics::GetInstance().GetContext().GetImmediateContext();
 
	ObjectPtr<DX11RenderTarget> dx_destination = ::resource_cast(destination);
 
 	// Shader setup
 
 	dx_destination->ClearDepth(*device_context);				// Clear the existing depth
 
 	scaling_shader_->SetInput(kSourceTexture, source);			// Source texture
 
 	dx_destination->Bind(*device_context);						// Destination texture
 
 	scaling_shader_->Bind(*device_context);						// Bind the shader
 		
 	// Render a quad
 
 	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
 	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 	
 	device_context->Draw(6, 0);									// Fire the rendering
 
 	// Unbind the material from the context
 
 	dx_destination->Unbind(*device_context);

	scaling_shader_->Unbind(*device_context);

}

size_t DX11FxScale::GetSize() const{

	return 0;

}
