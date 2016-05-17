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

struct CBSHFilter {

	unsigned int src_voxel_resolution;

	unsigned int dst_voxel_resolution;

	unsigned int dst_offset;

	unsigned int src_cascade;

	unsigned int dst_cascade;

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

	auto sh_unfiltered_pyramid = voxelization_.GetUnfilteredSHClipmap()->GetPyramid();
	auto sh_unfiltered_stack = voxelization_.GetUnfilteredSHClipmap()->GetStack();
	auto sh_filtered_pyramid = voxelization_.GetFilteredSHClipmap()->GetPyramid();
	auto sh_filtered_stack = voxelization_.GetFilteredSHClipmap()->GetStack();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ << &context;
	
	// Light accumulation setup

	auto&& resources = DX11Resources::GetInstance();

	gp_cache_ = resources.Load <IGPTexture2DCache, IGPTexture2DCache::Singleton>({});

	rt_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	light_shader_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\lighting.hlsl" });

	sh_filter_temp_ = resources.Load<IGPTexture3D, IGPTexture3D::FromDescription>({ sh_unfiltered_pyramid->GetWidth() >> 1,
																					sh_unfiltered_pyramid->GetHeight() >> 1,
																					sh_unfiltered_pyramid->GetDepth() >> 1,
																					1,
																					sh_unfiltered_pyramid->GetFormat()});

	shadow_atlas_ = make_unique<DX11VSMAtlas>(2048/*, 1*/, true);

	point_lights_ = new DX11StructuredArray(32, sizeof(PointLight));

	point_shadows_ = new DX11StructuredArray(32, sizeof(PointShadow));
	
	directional_lights_ = new DX11StructuredArray(32, sizeof(DirectionalLight));

	directional_shadows_ = new DX11StructuredArray(32, sizeof(DirectionalShadow));

	light_accumulation_parameters_ = new DX11StructuredBuffer(sizeof(LightAccumulationParameters));
	
	per_light_ = new DX11StructuredBuffer(sizeof(VSMPerLightCBuffer));

	cb_point_light_ = new DX11StructuredBuffer(sizeof(PointLight));
	
	cb_sh_filter = new DX11StructuredBuffer(sizeof(CBSHFilter));

	light_injection_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\inject_light.hlsl" });

	sh_stack_filter_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_filter.hlsl", { { "SH_STACK" } } });

	sh_pyramid_filter_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_filter.hlsl",{ { "SH_PYRAMID" } } });

	sh_stack_convert_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_convert.hlsl",{ { "SH_STACK" } } });

	sh_pyramid_convert_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_convert.hlsl",{ { "SH_PYRAMID" } } });

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
								sh_unfiltered_pyramid);

	light_injection_->SetOutput(DX11Voxelization::kUnfilteredSHStackTag, 
								sh_unfiltered_stack);

	light_injection_->SetInput("PerLight",
							   ObjectPtr<IStructuredBuffer>(per_light_));
	
	light_injection_->SetInput("CBPointLight",
							   ObjectPtr<IStructuredBuffer>(cb_point_light_));

	// Light filtering

	sh_stack_filter_->SetInput(DX11Voxelization::kVoxelizationTag,
							   voxelization_.GetVoxelizationParams());
	
	sh_stack_filter_->SetOutput(DX11Voxelization::kUnfilteredSHStackTag,
								sh_unfiltered_stack);

	sh_stack_filter_->SetInput("SHFilter",
							   ObjectPtr<IStructuredBuffer>(cb_sh_filter));
	
	sh_pyramid_filter_->SetInput(DX11Voxelization::kVoxelizationTag,
							     voxelization_.GetVoxelizationParams());

	sh_pyramid_filter_->SetInput("SHFilter",
							     ObjectPtr<IStructuredBuffer>(cb_sh_filter));

	// SH Convert setup - Stack

	sh_stack_convert_->SetInput(DX11Voxelization::kVoxelizationTag,
							    voxelization_.GetVoxelizationParams());
	
	sh_stack_convert_->SetInput("SHFilter",
								ObjectPtr<IStructuredBuffer>(cb_sh_filter));

	sh_stack_convert_->SetInput(DX11Voxelization::kUnfilteredSHStackTag,
								sh_unfiltered_stack->GetTexture());

	sh_stack_convert_->SetOutput(DX11Voxelization::kFilteredSHStackTag,
								 sh_filtered_stack);
	
	// SH Convert setup - Pyramid

	sh_pyramid_convert_->SetInput(DX11Voxelization::kVoxelizationTag,
							      voxelization_.GetVoxelizationParams());
		
	sh_pyramid_convert_->SetInput("SHFilter",
								  ObjectPtr<IStructuredBuffer>(cb_sh_filter));
	
	// Indirect lighting

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelizationTag, 
									 voxelization_.GetVoxelizationParams());

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelAddressTableTag,
									 voxelization_.GetVoxelAddressTable());

	indirect_light_shader_->SetInput(DX11Voxelization::kFilteredSHPyramidTag,
									 sh_filtered_pyramid->GetTexture());

	indirect_light_shader_->SetInput(DX11Voxelization::kFilteredSHStackTag,
									 sh_filtered_stack->GetTexture());

	indirect_light_shader_->SetInput(DX11Voxelization::kSHSampleTag,
									 voxelization_.GetSHSampler());

	indirect_light_shader_->SetInput(kLightParametersTag,
									 ObjectPtr<IStructuredBuffer>(light_accumulation_parameters_));

}

DX11DeferredRendererLighting::~DX11DeferredRendererLighting(){
	
	immediate_context_ = nullptr;
	
}

ObjectPtr<ITexture2D> DX11DeferredRendererLighting::AccumulateLight(const ObjectPtr<IRenderTarget>& gbuffer, const vector<VolumeComponent*>& lights, const FrameInfo& frame_info) {
		
	// Shadowmaps and optional light injection

	unsigned int point_lights_count;
	unsigned int directional_lights_count;

	UpdateShadowmaps(lights, frame_info, point_lights_count, directional_lights_count);
	
	// Shared CB setup

	auto light_accumulation_parameters = light_accumulation_parameters_->Lock<LightAccumulationParameters>();

	light_accumulation_parameters->camera_position = Math::ToVector3(frame_info.view_matrix.inverse().col(3));
	light_accumulation_parameters->inv_view_proj_matrix = frame_info.view_proj_matrix.inverse();
	light_accumulation_parameters->point_lights = point_lights_count;
	light_accumulation_parameters->directional_lights = directional_lights_count;

	light_accumulation_parameters_->Unlock();

	// Direct lighting

	AccumulateDirectLight(gbuffer, frame_info);

	// Indirect lighting

	if (frame_info.enable_global_illumination) {
        					   
		FilterIndirectLight();
		
		AccumulateIndirectLight(gbuffer, frame_info);

		return indirect_light_buffer_->GetTexture();

	}
	else {

	    return light_buffer_->GetTexture();

	}
    
}

void DX11DeferredRendererLighting::UpdateShadowmaps(const vector<VolumeComponent*>& lights, const FrameInfo &frame_info, unsigned int& point_lights_count, unsigned int& directional_lights_count)
{
	graphics_.PushEvent(L"Shadowmaps");

	shadow_atlas_->Reset();

	auto point_lights = point_lights_->Lock<PointLight>();
	auto point_shadows = point_shadows_->Lock<PointShadow>();
	auto directional_lights = directional_lights_->Lock<DirectionalLight>();
	auto directional_shadows = directional_shadows_->Lock<DirectionalShadow>();

	point_lights_count = 0;
	directional_lights_count = 0;

	for (auto&& node : lights) {

		// Point lights

		for (auto&& point_light : node->GetComponents<PointLightComponent>()) {

			UpdateLight(*frame_info.scene,
						point_light,
						point_lights[point_lights_count],
						point_shadows[point_lights_count],
						frame_info.enable_global_illumination);

			++point_lights_count;

		}

		// Directional lights

		for (auto&& directional_light : node->GetComponents<DirectionalLightComponent>()) {

			UpdateLight(*frame_info.scene,
						directional_light,
						frame_info.aspect_ratio,
						directional_lights[directional_lights_count],
						directional_shadows[directional_lights_count],
						frame_info.enable_global_illumination);

			++directional_lights_count;

		}

	}

	point_lights_->Unlock();
	point_shadows_->Unlock();
	directional_lights_->Unlock();
	directional_shadows_->Unlock();

	graphics_.PopEvent();

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

void DX11DeferredRendererLighting::AccumulateDirectLight(const ObjectPtr<IRenderTarget>& gbuffer, const FrameInfo &frame_info){

	graphics_.PushEvent(L"Direct light accumulation");

	gp_cache_->PushToCache(ObjectPtr<IGPTexture2D>(light_buffer_));			        // Release the previous light accumulation buffer, in case the resolution changed.

	light_buffer_ = gp_cache_->PopFromCache(frame_info.width,				        // Grab a new light accumulation buffer from the cache.
											frame_info.height,
											TextureFormat::RGB_FLOAT);

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

}

void DX11DeferredRendererLighting::AccumulateIndirectLight(const ObjectPtr<IRenderTarget>& gbuffer, const FrameInfo &frame_info) {

	graphics_.PushEvent(L"Indirect light accumulation");

	gp_cache_->PushToCache(ObjectPtr<IGPTexture2D>(indirect_light_buffer_));

	indirect_light_buffer_ = gp_cache_->PopFromCache(frame_info.width,				// Grab a new light accumulation buffer from the cache.
													 frame_info.height,
													 TextureFormat::RGB_FLOAT);

	bool check;

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

}

void DX11DeferredRendererLighting::FilterIndirectLight()
{

	CBSHFilter* cb_filter_params;

	int voxel_resolution = voxelization_.GetVoxelResolution();

	// Light filtering

	graphics_.PushEvent(L"Light filtering");

	auto unfiltered_sh_clipmap = voxelization_.GetUnfilteredSHClipmap();
	auto filtered_sh_clipmap = voxelization_.GetFilteredSHClipmap();
	
	// Filtering must be performed on the INT surface, since reading from a FLOAT is not supported by older architectures (Kepler and previous)

	if (voxelization_.GetVoxelCascades() > 0) {

		// Stack filtering - Copy the content of the N-th cascade to the (N-1) th one, up to the first one.

		for (int cascade_index = voxelization_.GetVoxelCascades() - 1; cascade_index > 0; --cascade_index) {

			cb_filter_params = cb_sh_filter->Lock<CBSHFilter>();

			cb_filter_params->src_voxel_resolution = voxel_resolution;						// Full cascade
			cb_filter_params->dst_voxel_resolution = voxel_resolution;						// Full cascade
			cb_filter_params->dst_offset = voxel_resolution >> 2;							// Center the downscaled cascade wrt the center of the next one.
			cb_filter_params->src_cascade = static_cast<unsigned int>(cascade_index);
			cb_filter_params->dst_cascade = static_cast<unsigned int>(cascade_index - 1);

			cb_sh_filter->Unlock();

			sh_stack_filter_->Dispatch(*immediate_context_,
									   voxel_resolution,
									   voxel_resolution,
									   voxel_resolution);

		}

		// Stack-to-pyramid filtering - Copy the content of the 1st cascade to the pyramid base.

		cb_filter_params = cb_sh_filter->Lock<CBSHFilter>();

		cb_filter_params->src_voxel_resolution = voxel_resolution;						// Full cascade
		cb_filter_params->dst_voxel_resolution = voxel_resolution;						// Full cascade
		cb_filter_params->dst_offset = voxel_resolution >> 2;							// Center the downscaled cascade wrt the center of the next one.
		cb_filter_params->src_cascade = 0;												// Least detailed level inside the stack.
		cb_filter_params->dst_cascade = 0;												// Pyramid do not use the cascade, but the MIPs

		cb_sh_filter->Unlock();

		sh_pyramid_filter_->SetInput(DX11Voxelization::kUnfilteredSHStackTag,
									 unfiltered_sh_clipmap->GetStack()->GetTexture());

		sh_pyramid_filter_->SetOutput(DX11Voxelization::kUnfilteredSHPyramidTag,
									  unfiltered_sh_clipmap->GetPyramid());

		sh_pyramid_filter_->Dispatch(*immediate_context_,
									 voxel_resolution,
									 voxel_resolution,
									 voxel_resolution);

		// SH stack conversion (3xINT32 -> FLOAT3)
		
		graphics_.PushEvent(L"SH stack conversion");

		cb_filter_params = cb_sh_filter->Lock<CBSHFilter>();

		cb_filter_params->src_voxel_resolution = voxel_resolution;			// Full cascade
		cb_filter_params->dst_voxel_resolution = voxel_resolution;			// Full cascade
		cb_filter_params->dst_offset = 0;									// Unused
		cb_filter_params->src_cascade = 0;									// 
		cb_filter_params->dst_cascade = 0;									// 

		cb_sh_filter->Unlock();

		sh_stack_convert_->Dispatch(*immediate_context_,
									voxel_resolution,
									voxel_resolution,
									voxel_resolution);

		graphics_.PopEvent();

	}

	// Pyramid filtering - Downscale the content of the N-th MIP to the (N+1)-th one

	int dst_resolution;
	ObjectPtr<ITexture3D> source_mip;
	ObjectPtr<ITexture3D> destination_mip = sh_filter_temp_->GetTexture();
	D3D11_BOX dest_region;

	for (unsigned int mip_index = 0; mip_index < unfiltered_sh_clipmap->GetPyramid()->GetMIPCount(); ++mip_index) {

		dst_resolution = voxel_resolution >> mip_index;

		source_mip = unfiltered_sh_clipmap->GetPyramid()->GetMIP(mip_index)->GetTexture();

		// SH pyramid conversion (3xINT32 -> FLOAT3)

		graphics_.PushEvent(L"SH pyramid conversion");

		cb_filter_params = cb_sh_filter->Lock<CBSHFilter>();

		cb_filter_params->src_voxel_resolution = dst_resolution;			// Current level resolution
		cb_filter_params->dst_voxel_resolution = dst_resolution;			// Current level resolution
		cb_filter_params->dst_offset = 0;									// Unused
		cb_filter_params->src_cascade = 0;									// 
		cb_filter_params->dst_cascade = 0;									// 

		cb_sh_filter->Unlock();

		sh_pyramid_convert_->SetInput(DX11Voxelization::kUnfilteredSHPyramidTag,
		 							  source_mip);

		sh_pyramid_convert_->SetOutput(DX11Voxelization::kFilteredSHPyramidTag,
									   filtered_sh_clipmap->GetPyramid()->GetMIP(mip_index));

		sh_pyramid_convert_->Dispatch(*immediate_context_,
									  dst_resolution,
									  dst_resolution,
									  dst_resolution);

		graphics_.PopEvent();

		// Downscale to the next MIP

		if (unfiltered_sh_clipmap->GetPyramid()->GetMIPCount() == mip_index + 1) {

			break;	// The last MIP is not downscaled

		}
		
		graphics_.PushEvent(L"SH pyramid downscale");

		// The current MIP level must be copied somewhere else since it seems that DX11 doesn't like to have an UAV and a SRV pointing to the same texture
		// (even if at different MIP levels) at the same time.

		cb_filter_params = cb_sh_filter->Lock<CBSHFilter>();

		cb_filter_params->src_voxel_resolution = dst_resolution;			// Current level resolution
		cb_filter_params->dst_voxel_resolution = dst_resolution >> 1;		// Next level resolution.
		cb_filter_params->dst_offset = 0;									// MIPs don't need to be centered.
		cb_filter_params->src_cascade = 0;									// Pyramid do not use the cascade, but the MIPs
		cb_filter_params->dst_cascade = 0;									// Pyramid do not use the cascade, but the MIPs

		cb_sh_filter->Unlock();
		
		sh_pyramid_filter_->SetInput(DX11Voxelization::kUnfilteredSHStackTag,
									 source_mip);

		sh_pyramid_filter_->SetOutput(DX11Voxelization::kUnfilteredSHPyramidTag,
									  sh_filter_temp_);

		sh_pyramid_filter_->Dispatch(*immediate_context_,
									 dst_resolution,
									 dst_resolution,
									 dst_resolution);

		dest_region.left = 0u;
		dest_region.top = 0u;
		dest_region.front = 0u;
		dest_region.right = sh_filter_temp_->GetWidth() >> mip_index;
		dest_region.bottom = sh_filter_temp_->GetHeight() >> mip_index;
		dest_region.back = sh_filter_temp_->GetDepth() >> mip_index;
		
		immediate_context_->CopySubresourceRegion(resource_cast(unfiltered_sh_clipmap->GetPyramid()->GetMIP(mip_index + 1)->GetTexture())->GetTexture().Get(),
												  mip_index + 1,
												  0,
												  0,
												  0,
												  resource_cast(sh_filter_temp_->GetTexture())->GetTexture().Get(),
												  0,
												  &dest_region);
				
		graphics_.PopEvent();
		
	}

	graphics_.PopEvent();

}
