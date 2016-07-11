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

namespace {

	Vector3i GetCascadeOffset(const Vector3f& center, float src_voxel_size) {

		float dst_voxel_size = 2.0f * src_voxel_size;

		auto src_center = center / src_voxel_size;
		auto dst_center = center / dst_voxel_size;

		auto offset = Vector3f(std::floor(src_center(0)) * src_voxel_size - std::floor(dst_center(0)) * dst_voxel_size,
							   std::floor(src_center(1)) * src_voxel_size - std::floor(dst_center(1)) * dst_voxel_size,
							   std::floor(src_center(2)) * src_voxel_size - std::floor(dst_center(2)) * dst_voxel_size);

		return Vector3i(std::floor(offset(0) / src_voxel_size),
						std::floor(offset(1) / src_voxel_size),
						std::floor(offset(2) / src_voxel_size));
		
	}

}

///////////////////////////////// DX11 DEFERRED RENDERER //////////////////////////////////

/// \brief Pixel shader constant buffer used to project the fragments to shadow space.
struct VSMPerLightCBuffer {

	Matrix4f light_matrix;								///< \brief Light world matrix used to transform from light space to world space.

	float near_plane;									///< \brief Near clipping plane.

	float far_plane;									///< \brief Far clipping plane.

	Vector2i padding;

};

struct CBSHFilter {

	Vector3i src_offset;								///< \brief Offset of the surface used as source of the filtering.

	int src_stride;										///< \brief Horizontal stride for each source SH band coefficient.

	Vector3i dst_offset;								///< \brief Offset of the surface used as destination of the filtering.

	int dst_stride;										///< \brief Horizontal stride for each destination SH band coefficient.

	Vector3i mip_offset;								///< \brief Offset to apply within a single filtered MIP.

	int padding;										

};

struct CBSHFilterStack {

	unsigned int dst_offset[3];							///< \brief Destination offset.

	unsigned int src_cascade;							///< \brief Source cascade.

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
const Tag DX11DeferredRendererLighting::kOperand1Tag = "gOperand1";
const Tag DX11DeferredRendererLighting::kOperand2Tag = "gOperand2";

DX11DeferredRendererLighting::DX11DeferredRendererLighting(DX11Voxelization& voxelization) :
graphics_(DX11Graphics::GetInstance()),
voxelization_(voxelization),
fx_downscale_(fx::FxScale::Parameters{}) {

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
	
	cb_sh_filter = new DX11StructuredBuffer(sizeof(CBSHFilter));

	light_injection_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\inject_light.hlsl" });

	sh_filter_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_filter.hlsl" });

	sh_convert_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_convert.hlsl" });

	indirect_light_shader_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\cone_tracing.hlsl" });
	
	indirect_light_sum_shader_ = new DX11Material(IMaterial::CompileFromFile{ L"Data\\Shaders\\common\\add_ps.hlsl" });

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::BILINEAR, 0 });

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

	light_injection_->SetOutput(DX11Voxelization::kRedSHTag,
								voxelization_.GetRedSHContribution());

	light_injection_->SetOutput(DX11Voxelization::kGreenSHTag,
								voxelization_.GetGreenSHContribution());

	light_injection_->SetOutput(DX11Voxelization::kBlueSHTag,
								voxelization_.GetBlueSHContribution());

	light_injection_->SetInput("PerLight",
							   ObjectPtr<IStructuredBuffer>(per_light_));
	
	light_injection_->SetInput("CBPointLight",
							   ObjectPtr<IStructuredBuffer>(cb_point_light_));

	// Spherical harmonics filtering

	sh_filter_->SetOutput(DX11Voxelization::kRedSHTag,
						  voxelization_.GetRedSHContribution());

	sh_filter_->SetOutput(DX11Voxelization::kGreenSHTag,
						  voxelization_.GetGreenSHContribution());

	sh_filter_->SetOutput(DX11Voxelization::kBlueSHTag,
						  voxelization_.GetBlueSHContribution());

	sh_filter_->SetInput("SHFilter",
						 ObjectPtr<IStructuredBuffer>(cb_sh_filter));
	
	// Spherical harmonics conversion

	sh_convert_->SetInput(DX11Voxelization::kRedSHTag,
						  voxelization_.GetRedSHContribution()->GetTexture());

	sh_convert_->SetInput(DX11Voxelization::kGreenSHTag,
						  voxelization_.GetGreenSHContribution()->GetTexture());

	sh_convert_->SetInput(DX11Voxelization::kBlueSHTag,
								voxelization_.GetBlueSHContribution()->GetTexture());
	
	sh_convert_->SetOutput(DX11Voxelization::kSHTag,
						   voxelization_.GetSH());
	
	// Indirect lighting

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelizationTag, 
									 voxelization_.GetVoxelizationParams());

	indirect_light_shader_->SetInput(DX11Voxelization::kVoxelAddressTableTag,
									 voxelization_.GetVoxelAddressTable());

	indirect_light_shader_->SetInput(DX11Voxelization::kSHTag,
									 voxelization_.GetSH()->GetTexture());

	indirect_light_shader_->SetInput(DX11Voxelization::kSHSampleTag,
									 voxelization_.GetSHSampler());

	indirect_light_shader_->SetInput(kLightParametersTag,
									 ObjectPtr<IStructuredBuffer>(light_accumulation_parameters_));

	// Indirect lighting

	indirect_light_sum_shader_->SetInput("gSampler",
										 ObjectPtr<ISampler>(sampler_));

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
        					   
		FilterIndirectLight(frame_info);
		
		AccumulateIndirectLight(gbuffer, frame_info);

		return (*indirect_light_buffer_)[0];

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

	auto indirect_light_accumulation_buffer_ = gp_cache_->PopFromCache(frame_info.width >> 1,		// Accumulate at half the resolution for performance reasons
																	   frame_info.height >> 1,
																	   TextureFormat::RGB_FLOAT);

	// Perform indirect light accumulation

	bool check;

	check = indirect_light_shader_->SetInput(kAlbedoEmissivityTag,
											 (*gbuffer)[0]);

	check = indirect_light_shader_->SetInput(kNormalShininessTag,
											 (*gbuffer)[1]);

	check = indirect_light_shader_->SetInput(kDepthStencilTag,
											 gbuffer->GetDepthBuffer());

	check = indirect_light_shader_->SetOutput(kIndirectLightBufferTag,
											  ObjectPtr<IGPTexture2D>(indirect_light_accumulation_buffer_));

	indirect_light_shader_->Dispatch(*immediate_context_,
									 indirect_light_accumulation_buffer_->GetWidth(),
									 indirect_light_accumulation_buffer_->GetHeight(),
									 1);

	// Upscale and add to the light buffer
	
	if (indirect_light_buffer_) {
	
		rt_cache_->PushToCache(indirect_light_buffer_);

	}

	indirect_light_buffer_ = rt_cache_->PopFromCache(frame_info.width,								// Final image result
													 frame_info.height,
													 { TextureFormat::RGB_FLOAT },
													 false);

	check = indirect_light_sum_shader_->SetInput(kOperand1Tag,
												 light_buffer_->GetTexture());

	check = indirect_light_sum_shader_->SetInput(kOperand2Tag,
												 indirect_light_accumulation_buffer_->GetTexture());

	resource_cast(indirect_light_buffer_)->Bind(*immediate_context_);

	indirect_light_sum_shader_->Bind(*immediate_context_);

	// Render a quad

	immediate_context_->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediate_context_->Draw(6, 0);

	indirect_light_sum_shader_->Unbind(*immediate_context_);

	resource_cast(indirect_light_buffer_)->Unbind(*immediate_context_);

	graphics_.PopEvent();

}

void DX11DeferredRendererLighting::FilterIndirectLight(const FrameInfo &frame_info)
{

	graphics_.PushEvent(L"Spherical harmonics filtering");

	int cascades = voxelization_.GetVoxelCascades();
	int voxel_resolution = voxelization_.GetVoxelResolution();

	Vector3f center = frame_info.camera->GetWorldTransform().translation();		// Center of the voxelization

	// Stack part
	
	for (int mip_index = -cascades; mip_index < 0; ++mip_index) {

		auto& cb = *cb_sh_filter->Lock<CBSHFilter>();

		auto offset = GetCascadeOffset(center, voxelization_.GetVoxelSize(mip_index));

		cb.src_offset = Vector3i(0, (-mip_index + 1) * voxel_resolution, 0) + offset;
		cb.src_stride = voxel_resolution;
		cb.dst_offset = Vector3i(0, (-mip_index + 0) * voxel_resolution, 0);
		cb.dst_stride = voxel_resolution;
		cb.mip_offset = Vector3i(voxel_resolution / 4, voxel_resolution / 4, voxel_resolution / 4) + offset;
		
		cb_sh_filter->Unlock();

		sh_filter_->Dispatch(*immediate_context_,
							 voxelization_.GetSH()->GetWidth(),				// VoxelResolution * #SHCoefficients
							 voxel_resolution,
							 voxel_resolution);

	}
	
	// Pyramid part
	
	// #BUG The border's color is wrong since a MIP would require a sample that is outside the boundaries
	// this causes some indirect color striping.
	// Remove the two highest MIP levels to reduce this defect.

	for (int mip_index = 0; mip_index < std::floor(std::log2(voxel_resolution)) - 2; ++mip_index) {

		auto& cb = *cb_sh_filter->Lock<CBSHFilter>();

		// Compensate from the source since the destination cascade may be misaligned
		cb.src_offset = Vector3i(0, voxel_resolution >> mip_index, 0) - GetCascadeOffset(center, voxelization_.GetVoxelSize(mip_index));
		cb.src_stride = voxel_resolution >> mip_index;
		cb.dst_offset = Vector3i(0, voxel_resolution >> (mip_index + 1), 0);
		cb.dst_stride = voxel_resolution >> (mip_index + 1);
		cb.mip_offset = Vector3i::Zero();
		
		cb_sh_filter->Unlock();

		sh_filter_->Dispatch(*immediate_context_,
							 voxelization_.GetSH()->GetWidth() >> mip_index,
							 voxel_resolution >> mip_index,
							 voxel_resolution >> mip_index);

	}

	graphics_.PopEvent();

	graphics_.PushEvent(L"Spherical harmonics conversion");

	sh_convert_->Dispatch(*immediate_context_,
						  voxelization_.GetSH()->GetWidth(),
						  voxelization_.GetSH()->GetHeight(),
						  voxelization_.GetSH()->GetDepth());

	graphics_.PopEvent();

}
