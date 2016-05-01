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

/// \brief Pixel shader constant buffer used to project the fragments to shadow space.
struct VSMPerLightCBuffer {

	Matrix4f light_matrix;								///< \brief Light world matrix used to transform from light space to world space.

	float near_plane;									///< \brief Near clipping plane.

	float far_plane;									///< \brief Far clipping plane.

	Vector2i padding;

};

const Tag DX11DeferredRendererLighting::kAlbedoEmissivityTag = "gAlbedoEmissivity";
const Tag DX11DeferredRendererLighting::kNormalShininessTag = "gNormalSpecularShininess";
const Tag DX11DeferredRendererLighting::kDepthStencilTag = "gDepthStencil";
const Tag DX11DeferredRendererLighting::kPointLightsTag = "gPointLights";
const Tag DX11DeferredRendererLighting::kDirectionalLightsTag = "gDirectionalLights";
const Tag DX11DeferredRendererLighting::kLightBufferTag = "gLightAccumulation";
const Tag DX11DeferredRendererLighting::kIndirectLightBufferTag = "gIndirectLight";
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

	auto&& resources = DX11Resources::GetInstance();

	gp_cache_ = resources.Load <IGPTexture2DCache, IGPTexture2DCache::Singleton>({});

	rt_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	light_shader_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\lighting.hlsl" });
	
	shadow_atlas_ = make_unique<DX11VSMAtlas>(2048/*, 1*/, true);

	point_lights_ = new DX11StructuredArray(32, sizeof(PointLight));

	point_shadows_ = new DX11StructuredArray(32, sizeof(PointShadow));
	
	directional_lights_ = new DX11StructuredArray(32, sizeof(DirectionalLight));

	directional_shadows_ = new DX11StructuredArray(32, sizeof(DirectionalShadow));

	light_accumulation_parameters_ = new DX11StructuredBuffer(sizeof(LightAccumulationParameters));
	
	per_light_ = new DX11StructuredBuffer(sizeof(VSMPerLightCBuffer));

	cb_point_light_ = new DX11StructuredBuffer(sizeof(PointLight));
	
	light_injection_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\inject_light.hlsl" });

	light_filtering_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\filter_light.hlsl" });

	indirect_light_shader_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\cone_tracing.hlsl" });
	
	bool check;

	// Light accumulation setup

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

	// Light injection

	light_injection_->SetInput(DX11Voxelization::kVoxelizationTag, 
							   voxelization_.GetVoxelizationParams());

	light_injection_->SetInput(DX11Voxelization::kVoxelAddressTableTag,
							   voxelization_.GetVoxelAddressTable());

	light_injection_->SetOutput(DX11Voxelization::kUnfilteredSHPyramidTag, 
								voxelization_.GetUnfilteredSHClipmap()->GetPyramid());

	light_injection_->SetOutput(DX11Voxelization::kUnfilteredSHStackTag, 
								voxelization_.GetUnfilteredSHClipmap()->GetStack());

	light_injection_->SetInput("PerLight",
							   ObjectPtr<IStructuredBuffer>(per_light_));
	
	light_injection_->SetInput("CBPointLight",
							   ObjectPtr<IStructuredBuffer>(cb_point_light_));

	// Light filtering setup

	light_filtering_->SetInput(DX11Voxelization::kVoxelizationTag,
							   voxelization_.GetVoxelizationParams());

	light_filtering_->SetInput(DX11Voxelization::kUnfilteredSHPyramidTag,
							   voxelization_.GetUnfilteredSHClipmap()->GetPyramid()->GetTexture());
	
	light_filtering_->SetInput(DX11Voxelization::kUnfilteredSHStackTag,
							   voxelization_.GetUnfilteredSHClipmap()->GetStack()->GetTexture());

	light_filtering_->SetOutput(DX11Voxelization::kFilteredSHPyramidTag,
								voxelization_.GetFilteredSHClipmap()->GetPyramid());
	
	light_filtering_->SetOutput(DX11Voxelization::kFilteredSHStackTag,
								voxelization_.GetFilteredSHClipmap()->GetStack());

	// Indirect lighting

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelizationTag, 
									 voxelization_.GetVoxelizationParams());

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelAddressTableTag,
									 voxelization_.GetVoxelAddressTable());

	indirect_light_shader_->SetInput(DX11Voxelization::kFilteredSHPyramidTag,
									 voxelization_.GetFilteredSHClipmap()->GetPyramid()->GetTexture());

	indirect_light_shader_->SetInput(DX11Voxelization::kFilteredSHStackTag,
									 voxelization_.GetFilteredSHClipmap()->GetStack()->GetTexture());

	indirect_light_shader_->SetInput(DX11Voxelization::kSHSampleTag,
									 voxelization_.GetSHSampler());

	indirect_light_shader_->SetInput(kLightParametersTag,
									 ObjectPtr<IStructuredBuffer>(light_accumulation_parameters_));



}

DX11DeferredRendererLighting::~DX11DeferredRendererLighting(){
	
	immediate_context_ = nullptr;
	
}

ObjectPtr<ITexture2D> DX11DeferredRendererLighting::AccumulateLight(const ObjectPtr<IRenderTarget>& gbuffer, const vector<VolumeComponent*>& lights, const FrameInfo& frame_info) {
	
	// Light accumulation setup

	gp_cache_->PushToCache(ObjectPtr<IGPTexture2D>(light_buffer_));			        // Release the previous light accumulation buffer, in case the resolution changed.
    gp_cache_->PushToCache(ObjectPtr<IGPTexture2D>(indirect_light_buffer_));

	light_buffer_ = gp_cache_->PopFromCache(frame_info.width,				        // Grab a new light accumulation buffer from the cache.
											frame_info.height,
											TextureFormat::RGB_FLOAT);

    indirect_light_buffer_ = gp_cache_->PopFromCache(frame_info.width,				// Grab a new light accumulation buffer from the cache.
											         frame_info.height,
											         TextureFormat::RGB_FLOAT);
		
	// Clear the shadow atlas from any existing shadowmap

	graphics_.PushEvent(L"Shadowmaps");

	shadow_atlas_->Reset();

	auto point_lights = point_lights_->Lock<PointLight>();
	auto point_shadows = point_shadows_->Lock<PointShadow>();
	auto directional_lights = directional_lights_->Lock<DirectionalLight>();
	auto directional_shadows = directional_shadows_->Lock<DirectionalShadow>();

	unsigned int point_light_index = 0;
	unsigned int directional_light_index = 0;

	for (auto&& node : lights) {

		for (auto&& point_light : node->GetComponents<PointLightComponent>()) {

			UpdateLight(*frame_info.scene,
						point_light,
						point_lights[point_light_index], 
						point_shadows[point_light_index],
						frame_info.enable_global_illumination);

			++point_light_index;

		}

		for (auto&& directional_light : node->GetComponents<DirectionalLightComponent>()) {

			UpdateLight(*frame_info.scene,
						directional_light,
						frame_info.aspect_ratio,
						directional_lights[directional_light_index],
						directional_shadows[directional_light_index],
						frame_info.enable_global_illumination);

			++directional_light_index;

		}

	}
	
	point_lights_->Unlock();
	point_shadows_->Unlock();
	directional_lights_->Unlock();
	directional_shadows_->Unlock();

	graphics_.PopEvent();

	if (frame_info.enable_global_illumination) {
		
		// Light filtering

		graphics_.PushEvent(L"Light filtering");

		light_filtering_->Dispatch(*immediate_context_,
								   voxelization_.GetVoxelResolution(),
								   voxelization_.GetVoxelResolution(),
								   voxelization_.GetVoxelResolution());
							   
		graphics_.PopEvent();

	}

	// Light accumulation

	graphics_.PushEvent(L"Direct light accumulation");

	auto light_accumulation_parameters = light_accumulation_parameters_->Lock<LightAccumulationParameters>();

	light_accumulation_parameters->camera_position = Math::ToVector3(frame_info.view_matrix.inverse().col(3));
	light_accumulation_parameters->inv_view_proj_matrix = frame_info.view_proj_matrix.inverse();
	light_accumulation_parameters->point_lights = point_light_index;
	light_accumulation_parameters->directional_lights = directional_light_index;
	
	light_accumulation_parameters_->Unlock();

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

	// Indirect lighting

	if (frame_info.enable_global_illumination) {

		graphics_.PushEvent(L"Indirect light accumulation");

		check = indirect_light_shader_->SetInput(kAlbedoEmissivityTag,
												 (*gbuffer)[0]);

		check = indirect_light_shader_->SetInput(kNormalShininessTag,
												 (*gbuffer)[1]);
	
		check = indirect_light_shader_->SetInput(kDepthStencilTag,
												 gbuffer->GetDepthBuffer());
	
		check = indirect_light_shader_->SetInput(kLightBufferTag,
												 light_buffer_->GetTexture());

        check = indirect_light_shader_->SetOutput(kIndirectLightBufferTag,
                                                  ObjectPtr<IGPTexture2D>(indirect_light_buffer_));

		indirect_light_shader_->Dispatch(*immediate_context_,
										 light_buffer_->GetWidth(),
										 light_buffer_->GetHeight(),
										 1);

		graphics_.PopEvent();

        return indirect_light_buffer_->GetTexture();

    }
    else{

	    return light_buffer_->GetTexture();

    }
    
}

void DX11DeferredRendererLighting::UpdateLight(const Scene& scene, const PointLightComponent& point_light, PointLight& light, PointShadow& shadow, bool light_injection) {

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
	
	if (light_injection) {

		// Light injection

		graphics_.PushEvent(L"Light injection");

		auto& per_light_front = *per_light_->Lock<VSMPerLightCBuffer>();

		per_light_front.near_plane = shadow.near_plane;
		per_light_front.far_plane = shadow.far_plane;
		per_light_front.light_matrix = shadow.light_view_matrix.inverse();

		per_light_->Unlock();

		auto point_light_buffer = cb_point_light_->Lock<PointLight>();

		*point_light_buffer = light;

		cb_point_light_->Unlock();

		light_injection_->SetInput(kVarianceShadowMapTag,
								   (*shadow_map)[0]);

		light_injection_->SetInput(kReflectiveShadowMapTag,
								   (*shadow_map)[1]);

		light_injection_->Dispatch(device_context,
								   shadow_map->GetWidth(),
								   shadow_map->GetHeight(),
								   1);

		graphics_.PopEvent();

	}

	// Cleanup

	rt_cache_->PushToCache(shadow_map);		// Save the texture for the next frame
	

}

void DX11DeferredRendererLighting::UpdateLight(const Scene& scene, const DirectionalLightComponent& directional_light, float /*aspect_ratio*/, DirectionalLight& light, DirectionalShadow& shadow, bool light_injection) {

	/*auto& graphics = DX11Graphics::GetInstance();*/

	/*auto& device_context = *graphics.GetContext().GetImmediateContext();*/

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
	if (light_injection) {

		graphics_.PushEvent(L"Light injection");

		light_injection_->SetInput(kVarianceShadowMapTag,
								   (*shadow_map)[0]);

		light_injection_->SetInput(kReflectiveShadowMapTag,
								   (*shadow_map)[1]);

		//light_injection_->Dispatch(device_context,
		//						   shadow_map->GetWidth(),
		//						   shadow_map->GetHeight(),
		//						   1);

		graphics_.PopEvent();

	}

	// Cleanup

	rt_cache_->PushToCache(shadow_map);		// Save the texture for the next frame

}