#include "dx11/dx11deferred_renderer.h"

#include "gimath.h"
#include "mesh.h"

#include "object.h"

#include "dx11/dx11.h"
#include "dx11/dx11render_target.h"
#include "dx11/dx11mesh.h"
#include "dx11/dx11shader.h"
#include "dx11/dx11shadow.h"

#include "windows/win_os.h"
#include "instance_builder.h"
#include "light_component.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;
using namespace ::gi_lib::windows;

namespace{
	
	/// \brief Compute the view-projection matrix given a camera and the aspect ratio of the target.
	Matrix4f ComputeViewProjectionMatrix(const CameraComponent& camera, float aspect_ratio){
	
		auto view_matrix = camera.GetViewTransform();

		Matrix4f projection_matrix;

		// Hey this method should totally become a member method of the camera component!
		// NOPE! DirectX and OpenGL calculate the projection differently (mostly because of the near plane which, in the first, case goes from 0 to 1, in the latter goes from -1 to 1).
		// TODO: Actually we may just keep the OGL notation and let Z(dx) = Z(gl)/2 + 0.5 while working on DX
		
		if (camera.GetProjectionType() == ProjectionType::Perspective){
						
			projection_matrix = ComputePerspectiveProjectionLH(camera.GetFieldOfView(),
															   aspect_ratio,
															   camera.GetMinimumDistance(),
															   camera.GetMaximumDistance());

		}
		else if (camera.GetProjectionType() == ProjectionType::Ortographic){

			projection_matrix = ComputeOrthographicProjectionLH(camera.GetOrthoSize() * aspect_ratio,
																camera.GetOrthoSize(),
																camera.GetMinimumDistance(),
																camera.GetMaximumDistance());

		}
		else{

			THROW(L"Unsupported projection mode!");

		}
		
		return (projection_matrix * view_matrix).matrix();

	}

	/// \brief Compute the visible nodes inside a given hierachy given the camera and the aspect ratio of the target.
	vector<VolumeComponent*> ComputeVisibleNodes(const IVolumeHierarchy& volume_hierarchy, const CameraComponent& camera, float aspect_ratio){

		// Basic frustum culling

		auto camera_frustum = camera.GetViewFrustum(aspect_ratio);

		return volume_hierarchy.GetIntersections(camera_frustum);											// Updates the view frustum according to the output ratio.
		
	}

}

///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

const Tag DX11DeferredRendererMaterial::kDiffuseMapTag = "gDiffuseMap";
const Tag DX11DeferredRendererMaterial::kDiffuseSampler = "gDiffuseSampler";
const Tag DX11DeferredRendererMaterial::kPerObjectTag = "PerObject";

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const CompileFromFile& args) :
material_(new DX11Material(args)){

	auto& resources = DX11Graphics::GetInstance().GetResources();

	per_object_cbuffer_ = resources.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(VSPerObjectBuffer) });

	material_->SetInput(kPerObjectTag, ObjectPtr<IStructuredBuffer>(per_object_cbuffer_));

}

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const ObjectPtr<DX11Material>& base_material) :
material_(base_material->Instantiate()){

	auto& resources = DX11Graphics::GetInstance().GetResources();

	per_object_cbuffer_ = resources.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(VSPerObjectBuffer) });

	material_->SetInput(kPerObjectTag, ObjectPtr<IStructuredBuffer>(per_object_cbuffer_));

}

ObjectPtr<DeferredRendererMaterial> DX11DeferredRendererMaterial::Instantiate() const{

	return new DX11DeferredRendererMaterial(material_);		

}

void DX11DeferredRendererMaterial::SetMatrix(const Affine3f& world, const Matrix4f& view_projection){

	// Lock
	auto& buffer = *per_object_cbuffer_->Lock<VSPerObjectBuffer>();

	// Update

	buffer.world = world.matrix();
	buffer.world_view_proj = (view_projection * world).matrix();

	// Unlock

	per_object_cbuffer_->Unlock();
	
}

///////////////////////////////// DX11 TILED DEFERRED RENDERER //////////////////////////////////

const Tag DX11DeferredRenderer::kAlbedoEmissivityTag = "gAlbedoEmissivity";
const Tag DX11DeferredRenderer::kNormalShininessTag = "gNormalSpecularShininess";
const Tag DX11DeferredRenderer::kDepthStencilTag = "gDepthStencil";
const Tag DX11DeferredRenderer::kPointLightsTag = "gPointLights";
const Tag DX11DeferredRenderer::kDirectionalLightsTag = "gDirectionalLights";
const Tag DX11DeferredRenderer::kLightBufferTag = "gLightAccumulation";
const Tag DX11DeferredRenderer::kLightParametersTag = "gParameters";
const Tag DX11DeferredRenderer::kVSMShadowAtlasTag = "gVSMShadowAtlas";
const Tag DX11DeferredRenderer::kVSMSamplerTag = "gVSMSampler";
const Tag DX11DeferredRenderer::kPointShadowsTag = "gPointShadows";
const Tag DX11DeferredRenderer::kDirectionalShadowsTag = "gDirectionalShadows";

DX11DeferredRenderer::DX11DeferredRenderer(const RendererConstructionArgs& arguments) :
DeferredRenderer(arguments.scene),
fx_bloom_(1.0f, 1.67f, Vector2f(0.5f, 0.5f)),
fx_tonemap_(0.5f){

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ << &context;

	// Create the depth stencil state

	D3D11_DEPTH_STENCIL_DESC depth_state_desc;

	ID3D11DepthStencilState* depth_state;
	
	depth_state_desc.DepthEnable = true;
	depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_state_desc.DepthFunc = D3D11_COMPARISON_LESS;
	depth_state_desc.StencilEnable = false;
	depth_state_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depth_state_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	
	depth_state_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	depth_state_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	device.CreateDepthStencilState(&depth_state_desc,
								   &depth_state);

	depth_state_ << &depth_state;

	ZeroMemory(&depth_state_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	device.CreateDepthStencilState(&depth_state_desc,
								   &depth_state);

	disable_depth_test_ << &depth_state;

	// Create the blend state

	D3D11_BLEND_DESC blend_state_desc;

	ID3D11BlendState* blend_state;
	
	blend_state_desc.AlphaToCoverageEnable = false;
	blend_state_desc.IndependentBlendEnable = false;

	blend_state_desc.RenderTarget[0].BlendEnable = false;
	blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device.CreateBlendState(&blend_state_desc,
							&blend_state);

	blend_state_ << &blend_state;

	// Create the raster state.

	D3D11_RASTERIZER_DESC rasterizer_state_desc;

	ID3D11RasterizerState* rasterizer_state;

	rasterizer_state_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_state_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_state_desc.FrontCounterClockwise = false;
	rasterizer_state_desc.DepthBias = 0;
	rasterizer_state_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_state_desc.DepthBiasClamp = 0.0f;
	rasterizer_state_desc.DepthClipEnable = true;
	rasterizer_state_desc.ScissorEnable = false;
	rasterizer_state_desc.MultisampleEnable = false;
	rasterizer_state_desc.AntialiasedLineEnable = false;

	device.CreateRasterizerState(&rasterizer_state_desc,
								 &rasterizer_state);

	rasterizer_state_ << &rasterizer_state;
	
	// Move the stuffs below somewhere else

	auto&& app = Application::GetInstance();

	// Light accumulation setup

	light_shader_ = DX11Resources::GetInstance().Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\lighting.hlsl" });

	shadow_atlas_ = make_unique<DX11VSMAtlas>(2048, 1, true);

	point_lights_ = new DX11StructuredArray(32, sizeof(PointLight));

	point_shadows_ = new DX11StructuredArray(32, sizeof(PointShadow));
	
	directional_lights_ = new DX11StructuredArray(32, sizeof(DirectionalLight));

	directional_shadows_ = new DX11StructuredArray(32, sizeof(DirectionalShadow));

	light_accumulation_parameters_ = new DX11StructuredBuffer(sizeof(LightAccumulationParameters));
		
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
									ObjectPtr<ITexture2DArray>(shadow_atlas_->GetAtlas()));
	
}

DX11DeferredRenderer::~DX11DeferredRenderer(){
	
	immediate_context_ = nullptr;
	depth_state_ = nullptr;
	blend_state_ = nullptr;
	rasterizer_state_ = nullptr;
	disable_depth_test_ = nullptr;
	
}

ObjectPtr<ITexture2D> DX11DeferredRenderer::Draw(unsigned int width, unsigned int height){
	
	// Draws only if there's a camera

	auto main_camera = GetScene().GetMainCamera();

	if (main_camera){

		FrameInfo frame_info;

		frame_info.scene = &GetScene();
		frame_info.camera = main_camera;
		frame_info.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		frame_info.width = width;
		frame_info.height = height;
		frame_info.view_proj_matrix = ComputeViewProjectionMatrix(*main_camera, frame_info.aspect_ratio);

		DrawGBuffer(frame_info);							// Scene -> GBuffer
		
		ComputeLighting(frame_info);						// Scene, GBuffer, DepthBuffer -> LightBuffer

		ComputePostProcess(frame_info);						// LightBuffer -> Output

	}

	// Cleanup
	immediate_context_->ClearState();
	
	// Return the unexposed buffer
	return tonemap_output_->GetTexture();

}

// GBuffer

void DX11DeferredRenderer::DrawGBuffer(const FrameInfo& frame_info){

	// Setup the GBuffer and bind it to the immediate context.
	
	BindGBuffer(frame_info);

	// Draw the visible nodes

	DrawNodes(ComputeVisibleNodes(frame_info.scene->GetMeshHierarchy(), *frame_info.camera, frame_info.aspect_ratio), 
			  frame_info);
	
	// Cleanup

	gbuffer_->Unbind(*immediate_context_);

}

void DX11DeferredRenderer::BindGBuffer(const FrameInfo& frame_info){

	// Rasterizer state setup

	immediate_context_->RSSetState(rasterizer_state_.Get());

	// Setup of the output merger - This should be defined per render section actually but for now only opaque geometry with no fancy stencil effects is supported.

	immediate_context_->OMSetDepthStencilState(depth_state_.Get(),
											   0);

	immediate_context_->OMSetBlendState(blend_state_.Get(),
										0,
										0xFFFFFFFF);


	// Setup of the render target surface (GBuffer)

	if (!gbuffer_){

		// Lazy initialization

		gbuffer_ = new DX11RenderTarget(IRenderTarget::FromDescription{ frame_info.width,
																		frame_info.height,
																		{ TextureFormat::RGBA_HALF,
																		  TextureFormat::RGBA_HALF } } );

	}
	else{

		// GBuffer resize (will actually resize the target only if the size has been changed since the last frame)

		gbuffer_->Resize(frame_info.width,
						 frame_info.height);

	}

	static const Color kSkyColor = Color{ 0.66f, 2.05f, 3.96f, 1.0f };

	gbuffer_->ClearTargets(*immediate_context_, kSkyColor);
	
	gbuffer_->ClearDepth(*immediate_context_);

	// Bind the gbuffer to the immediate context

	gbuffer_->Bind(*immediate_context_);

}

void DX11DeferredRenderer::DrawNodes(const vector<VolumeComponent*>& meshes, const FrameInfo& frame_info){

	ObjectPtr<DX11Mesh> mesh;
	ObjectPtr<DX11DeferredRendererMaterial> material;

	// Trivial solution: cycle through each node and for each drawable component draws the subsets.

	// TODO: Implement some batching strategy here!

	for (auto&& node : meshes){

		for (auto&& drawable : node->GetComponents<AspectComponent<DeferredRendererMaterial>>()){

			// Bind the mesh

			mesh = drawable.GetMesh();

			mesh->Bind(*immediate_context_);

			// For each subset

			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index){
				
				// Bind the subset material

				material = drawable.GetMaterial(subset_index);

				material->SetMatrix(drawable.GetWorldTransform(),
									frame_info.view_proj_matrix);

				material->Bind(*immediate_context_);

				// Draw	the subset
				mesh->DrawSubset(*immediate_context_, 
								 subset_index);

			}

		}

	}

}

// Lighting

void DX11DeferredRenderer::ComputeLighting(const FrameInfo& frame_info){

	// Lazy setup and resize of the light accumulation buffer

	if (!light_buffer_ ||
		light_buffer_->GetWidth() != frame_info.width ||
		light_buffer_->GetHeight() != frame_info.height){

		light_buffer_ = new DX11GPTexture2D(IGPTexture2D::FromDescription{ frame_info.width,
																		   frame_info.height,
																		   1,
																		   TextureFormat::RGB_FLOAT });
		
	}

	// Accumulate the visible lights
	auto&& visible_lights = ComputeVisibleNodes(frame_info.scene->GetLightHierarchy(), *frame_info.camera, frame_info.aspect_ratio);


	AccumulateLight(visible_lights,
					frame_info);

}

void DX11DeferredRenderer::AccumulateLight(const vector<VolumeComponent*>& lights, const FrameInfo& frame_info) {
	
	// Clear the shadow atlas from any existing shadowmap
	shadow_atlas_->Begin();

	auto point_lights = point_lights_->Lock<PointLight>();
	
	auto point_shadows = point_shadows_->Lock<PointShadow>();

	auto directional_lights = directional_lights_->Lock<DirectionalLight>();
	
	auto directional_shadows = directional_shadows_->Lock<DirectionalShadow>();

	auto light_accumulation_parameters = light_accumulation_parameters_->Lock<LightAccumulationParameters>();

	unsigned int point_light_index = 0;
	unsigned int directional_light_index = 0;

	for (auto&& node : lights) {

		for (auto&& point_light : node->GetComponents<PointLightComponent>()) {

			UpdateLight(point_light, 
						point_lights[point_light_index], 
						point_shadows[point_light_index]);

			++point_light_index;

		}

		for (auto&& directional_light : node->GetComponents<DirectionalLightComponent>()) {

			UpdateLight(directional_light,
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
	
	// Finalize the shadows
	
	shadow_atlas_->Commit();

	// These entities may change from frame to frame
	
	bool check;

	check = light_shader_->SetInput(kAlbedoEmissivityTag,
									(*gbuffer_)[0]);

	check = light_shader_->SetInput(kNormalShininessTag,
									(*gbuffer_)[1]);
	
	check = light_shader_->SetInput(kDepthStencilTag,
									gbuffer_->GetDepthBuffer());
	
	check = light_shader_->SetOutput(kLightBufferTag,
									 ObjectPtr<IGPTexture2D>(light_buffer_));

	// Actual light computation

	light_shader_->Dispatch(*immediate_context_,			// Dispatch one thread for each GBuffer's pixel
							light_buffer_->GetWidth(),
							light_buffer_->GetHeight(),
							1);

}

void DX11DeferredRenderer::UpdateLight(const PointLightComponent& point_light, PointLight& light, PointShadow& shadow) {

	// Light

	light.position = Math::ToVector4(point_light.GetPosition(), 1.0f);
	light.color = point_light.GetColor().ToVector4f();
	light.kc = point_light.GetConstantFactor();
	light.kl = point_light.GetLinearFactor();
	light.kq = point_light.GetQuadraticFactor();
	light.cutoff = point_light.GetCutoff();

	// Shadow map calculation

	shadow_atlas_->ComputeShadowmap(point_light, 
									GetScene(), 
									shadow);
	
}

void DX11DeferredRenderer::UpdateLight(const DirectionalLightComponent& directional_light, float aspect_ratio, DirectionalLight& light, DirectionalShadow& shadow) {

	// Light

	light.direction = Math::ToVector4(directional_light.GetDirection(), 1.0f);
	light.color = directional_light.GetColor().ToVector4f();

	// Shadow
	shadow_atlas_->ComputeShadowmap(directional_light,
									GetScene(),
									shadow,
									aspect_ratio);

}

// Post processing

void DX11DeferredRenderer::ComputePostProcess(const FrameInfo& frame_info){

	// LightBuffer == [Bloom] ==> Unexposed ==> [Tonemap] ==> Output

	// Lazy initialization and resize of the bloom and tonemap surfaces

	if (!bloom_output_ ||
		 bloom_output_->GetWidth() != frame_info.width ||
		 bloom_output_->GetHeight() != frame_info.height) {

		bloom_output_ = new DX11RenderTarget(IRenderTarget::FromDescription{ frame_info.width,
																			 frame_info.height,
																			 { TextureFormat::RGB_FLOAT } });

		tonemap_output_ = new DX11GPTexture2D(IGPTexture2D::FromDescription{ frame_info.width,
																			 frame_info.height,
																			 1,
																			 TextureFormat::RGBA_HALF_UNORM });

	}

	// Bloom

	fx_bloom_.Process(light_buffer_->GetTexture(),
					  bloom_output_);

	
	// Tonemap

	fx_tonemap_.Process((*bloom_output_)[0],
						tonemap_output_);
	
}