#include "dx11/dx11deferred_renderer_lighting.h"

#include "tag.h"
#include "light_component.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11gpgpu.h"
#include "dx11/dx11shadow.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::windows;
using namespace ::gi_lib::dx11;

///////////////////////////////// DX11 DEFERRED RENDERER //////////////////////////////////

const Tag DX11DeferredRendererLighting::kAlbedoEmissivityTag = "gAlbedoEmissivity";
const Tag DX11DeferredRendererLighting::kNormalShininessTag = "gNormalSpecularShininess";
const Tag DX11DeferredRendererLighting::kDepthStencilTag = "gDepthStencil";
const Tag DX11DeferredRendererLighting::kPointLightsTag = "gPointLights";
const Tag DX11DeferredRendererLighting::kDirectionalLightsTag = "gDirectionalLights";
const Tag DX11DeferredRendererLighting::kLightBufferTag = "gLightAccumulation";
const Tag DX11DeferredRendererLighting::kLightParametersTag = "gParameters";
const Tag DX11DeferredRendererLighting::kVSMShadowAtlasTag = "gVSMShadowAtlas";
const Tag DX11DeferredRendererLighting::kVSMSamplerTag = "gVSMSampler";
const Tag DX11DeferredRendererLighting::kPointShadowsTag = "gPointShadows";
const Tag DX11DeferredRendererLighting::kDirectionalShadowsTag = "gDirectionalShadows";
const Tag DX11DeferredRendererLighting::kReflectiveShadowMapTag = "gRSM";
const Tag DX11DeferredRendererLighting::kVarianceShadowMapTag = "gVSM";

DX11DeferredRendererLighting::DX11DeferredRendererLighting(DX11Voxelization& voxelization) :
graphics_(DX11Graphics::GetInstance()),
voxelization_(voxelization){

	auto&& device = *graphics_.GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ << &context;
	
	// Light accumulation setup

	auto&& app = Application::GetInstance();

	auto&& resources = DX11Resources::GetInstance();

	gp_cache_ = resources.Load <IGPTexture2DCache, IGPTexture2DCache::Singleton>({});

	rt_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	light_shader_ = resources.Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\lighting.hlsl" });
	
	shadow_atlas_ = make_unique<DX11VSMAtlas>(2048/*, 1*/, true);

	point_lights_ = new DX11StructuredArray(32, sizeof(PointLight));

	point_shadows_ = new DX11StructuredArray(32, sizeof(PointShadow));
	
	directional_lights_ = new DX11StructuredArray(32, sizeof(DirectionalLight));

	directional_shadows_ = new DX11StructuredArray(32, sizeof(DirectionalShadow));

	light_accumulation_parameters_ = new DX11StructuredBuffer(sizeof(LightAccumulationParameters));
		
	light_injection_ = resources.Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\voxel\\inject_light.hlsl" });

	// One-time setup

	bool check;

	check = light_shader_->SetInput(kLightParametersTag,
									ObjectPtr<IStructuredBuffer>(light_accumulation_parameters_));

	check = light_shader_->SetInput(kPointLightsTag,
									ObjectPtr<IStructuredArray>(point_lights_));

	check = light_shader_->SetInput(kPointShadowsTag,
									ObjectPtr<IStructuredArray>(point_shadows_));

	check = light_shader_->SetInput(kDirectionalLightsTag,
									ObjectPtr<IStructuredArray>(directional_lights_));

	check = light_shader_->SetInput(kDirectionalShadowsTag,
									ObjectPtr<IStructuredArray>(directional_shadows_));

	check = light_shader_->SetInput(kVSMSamplerTag,
									ObjectPtr<ISampler>(shadow_atlas_->GetSampler()));

	check = light_shader_->SetInput(kVSMShadowAtlasTag,
									ObjectPtr<ITexture2D>(shadow_atlas_->GetAtlas()));
	
}

DX11DeferredRendererLighting::~DX11DeferredRendererLighting(){
	
	immediate_context_ = nullptr;
	
}

ObjectPtr<ITexture2D> DX11DeferredRendererLighting::AccumulateLight(const ObjectPtr<IRenderTarget>& gbuffer, const vector<VolumeComponent*>& lights, const FrameInfo& frame_info) {
	
	// Light accumulation setup

	gp_cache_->PushToCache(ObjectPtr<IGPTexture2D>(light_buffer_));			// Release the previous light accumulation buffer, in case the resolution changed.

	light_buffer_ = gp_cache_->PopFromCache(frame_info.width,				// Grab a new light accumulation buffer from the cache.
											frame_info.height,
											TextureFormat::RGB_FLOAT);
	
	// SH setup

	light_injection_->SetOutput(DX11Voxelization::kRedSH01Tag, voxelization_.GetSH(0));
	light_injection_->SetOutput(DX11Voxelization::kGreenSH01Tag, voxelization_.GetSH(1));
	light_injection_->SetOutput(DX11Voxelization::kBlueSH01Tag, voxelization_.GetSH(2));
	
	// Clear the shadow atlas from any existing shadowmap

	graphics_.PushEvent(L"Shadowmap + Light Injection");

	shadow_atlas_->Reset();

	auto point_lights = point_lights_->Lock<PointLight>();
	
	auto point_shadows = point_shadows_->Lock<PointShadow>();

	auto directional_lights = directional_lights_->Lock<DirectionalLight>();
	
	auto directional_shadows = directional_shadows_->Lock<DirectionalShadow>();

	auto light_accumulation_parameters = light_accumulation_parameters_->Lock<LightAccumulationParameters>();

	unsigned int point_light_index = 0;
	unsigned int directional_light_index = 0;

	for (auto&& node : lights) {

		for (auto&& point_light : node->GetComponents<PointLightComponent>()) {

			UpdateLight(*frame_info.scene,
						point_light,
						point_lights[point_light_index], 
						point_shadows[point_light_index]);

			++point_light_index;

		}

		for (auto&& directional_light : node->GetComponents<DirectionalLightComponent>()) {

			UpdateLight(*frame_info.scene,
						directional_light,
						frame_info.aspect_ratio,
						directional_lights[directional_light_index],
						directional_shadows[directional_light_index]);

			++directional_light_index;

		}

	}

	light_accumulation_parameters->camera_position = frame_info.camera->GetTransformComponent().GetPosition();
	light_accumulation_parameters->inv_view_proj_matrix = frame_info.view_proj_matrix.inverse();
	light_accumulation_parameters->point_lights = point_light_index;
	light_accumulation_parameters->directional_lights = directional_light_index;

	point_lights_->Unlock();
	point_shadows_->Unlock();
	directional_lights_->Unlock();
	directional_shadows_->Unlock();
	light_accumulation_parameters_->Unlock();
	
	graphics_.PopEvent();

	// These entities may change from frame to frame
	
	graphics_.PushEvent(L"Light accumulation");

	bool check;

	check = light_shader_->SetInput(kAlbedoEmissivityTag,
									(*gbuffer)[0]);

	check = light_shader_->SetInput(kNormalShininessTag,
									(*gbuffer)[1]);
	
	check = light_shader_->SetInput(kDepthStencilTag,
									gbuffer->GetDepthBuffer());
	
	check = light_shader_->SetOutput(kLightBufferTag,
									 ObjectPtr<IGPTexture2D>(light_buffer_));

	// Actual light computation

	light_shader_->Dispatch(*immediate_context_,			// Dispatch one thread for each GBuffer's pixel
							light_buffer_->GetWidth(),
							light_buffer_->GetHeight(),
							1);

	graphics_.PopEvent();

	return light_buffer_->GetTexture();

}

void DX11DeferredRendererLighting::UpdateLight(const Scene& scene, const PointLightComponent& point_light, PointLight& light, PointShadow& shadow) {

	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetContext().GetImmediateContext();

	// Light

	light.position = Math::ToVector4(point_light.GetPosition(), 1.0f);
	light.color = point_light.GetColor().ToVector4f();
	light.kc = point_light.GetConstantFactor();
	light.kl = point_light.GetLinearFactor();
	light.kq = point_light.GetQuadraticFactor();
	light.cutoff = point_light.GetCutoff();

	// Shadow map calculation

	ObjectPtr<IRenderTarget> shadow_map;

	shadow_atlas_->ComputeShadowmap(point_light, 
									scene, 
									shadow,
									&shadow_map);
	
	// Light injection

	graphics_.PushEvent(L"Light injection");

	light_injection_->SetInput(kVarianceShadowMapTag,
							   (*shadow_map)[0]);

	light_injection_->SetInput(kReflectiveShadowMapTag,
							   (*shadow_map)[1]);

	light_injection_->Dispatch(device_context,
							   shadow_map->GetWidth(),
							   shadow_map->GetHeight(),
							   1);

	graphics_.PopEvent();

	// Cleanup

	rt_cache_->PushToCache(shadow_map);		// Save the texture for the next frame
	

}

void DX11DeferredRendererLighting::UpdateLight(const Scene& scene, const DirectionalLightComponent& directional_light, float aspect_ratio, DirectionalLight& light, DirectionalShadow& shadow) {

	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetContext().GetImmediateContext();

	// Light

	light.direction = Math::ToVector4(directional_light.GetDirection(), 1.0f);
	light.color = directional_light.GetColor().ToVector4f();

	// Shadow map calculation
	
	ObjectPtr<IRenderTarget> shadow_map;

	shadow_atlas_->ComputeShadowmap(directional_light,
									scene,
									shadow,
									&shadow_map);

	// Light injection

	graphics_.PushEvent(L"Light injection");

	light_injection_->SetInput(kVarianceShadowMapTag,
							   (*shadow_map)[0]);

	light_injection_->SetInput(kReflectiveShadowMapTag,
							   (*shadow_map)[1]);

	light_injection_->Dispatch(device_context,
							   shadow_map->GetWidth(),
							   shadow_map->GetHeight(),
							   1);

	graphics_.PopEvent();

	// Cleanup

	rt_cache_->PushToCache(shadow_map);		// Save the texture for the next frame

}