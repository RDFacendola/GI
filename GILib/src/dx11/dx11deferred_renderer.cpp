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
#include "dx11/dx11buffer.h"

#include "dx11/fx/dx11fx_postprocess.h"
#include "dx11/fx/dx11fx_transform.h"
#include "dx11/fx/dx11fx_filter.h"
#include "dx11/fx/dx11fx_image.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::fx;
using namespace ::gi_lib::dx11;
using namespace ::gi_lib::windows;

namespace{
	
	/// \brief Compute the view-projection matrix given a camera and the aspect ratio of the target.
	Matrix4f ComputeViewProjectionMatrix(const CameraComponent& camera, float aspect_ratio){
	
		auto view_matrix = camera.GetViewTransform();

		Matrix4f projection_matrix;

		// Hey this method should totally become a member method of the camera component!
		// NOPE! DirectX and OpenGL calculate the projection differently (mostly because of the near plane which, in the first case, goes from 0 to 1, in the latter goes from -1 to 1).
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

	/// \brief Compute the visible nodes inside a given hierarchy given the camera and the aspect ratio of the target.
	vector<VolumeComponent*> ComputeVisibleNodes(const IVolumeHierarchy& volume_hierarchy, const CameraComponent& camera, float aspect_ratio){

		// Basic frustum culling

		auto camera_frustum = camera.GetViewFrustum(aspect_ratio);

		return volume_hierarchy.GetIntersections(camera_frustum);											// Updates the view frustum according to the output ratio.
		
	}

}

///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

const Tag DX11DeferredRendererMaterial::kShaderParameters = "PerObject";

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const CompileFromFile& args) :
material_(new DX11Material(args)){

	auto& resources = DX11Graphics::GetInstance().GetResources();

	shader_parameters_ = resources.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(ShaderParameters) });

	material_->SetInput(kShaderParameters, ObjectPtr<IStructuredBuffer>(shader_parameters_));

}

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const ObjectPtr<DX11Material>& base_material) :
material_(base_material->Instantiate()){

	auto& resources = DX11Graphics::GetInstance().GetResources();

	shader_parameters_ = resources.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(ShaderParameters) });

	material_->SetInput(kShaderParameters, ObjectPtr<IStructuredBuffer>(shader_parameters_));

}

ObjectPtr<DeferredRendererMaterial> DX11DeferredRendererMaterial::Instantiate() const{

	return new DX11DeferredRendererMaterial(material_);		

}

void DX11DeferredRendererMaterial::SetMatrix(const Affine3f& world, const Matrix4f& view_projection){

	auto& buffer = *shader_parameters_->Lock<ShaderParameters>();

	// Update

	buffer.world = world.matrix();
	buffer.world_view_proj = (view_projection * world).matrix();

	shader_parameters_->Unlock();
	
}

///////////////////////////////// DX11 DEFERRED RENDERER //////////////////////////////////

DX11DeferredRenderer::DX11DeferredRenderer(const RendererConstructionArgs& arguments) :
DeferredRenderer(arguments.scene),
graphics_(DX11Graphics::GetInstance()){

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ << &context;

	// GBuffer setup

	auto&& resources = DX11Resources::GetInstance();

	rt_cache_ = resources.Load <IRenderTargetCache, IRenderTargetCache::Singleton>({});
	
	// Voxel setup

	voxelization_ = std::make_unique<DX11Voxelization>(*this, 800.0f, 64, 4);

	// Lighting setup

	lighting_ = std::make_unique<DX11DeferredRendererLighting>(*voxelization_);

}

DX11DeferredRenderer::~DX11DeferredRenderer(){
	
	immediate_context_ = nullptr;
	lighting_ = nullptr;
	voxelization_ = nullptr;
	
}

Matrix4f DX11DeferredRenderer::GetViewProjectionMatrix(float aspect_ratio) const {

	auto main_camera = GetScene().GetMainCamera();

	if (main_camera) {

		return ComputeViewProjectionMatrix(*main_camera, aspect_ratio);

	}
	else {

		return Matrix4f::Identity();

	}

}

ObjectPtr<ITexture2D> DX11DeferredRenderer::Draw(const Time& time, unsigned int width, unsigned int height){
	
	graphics_.PushEvent(L"Frame");

	auto& context = DX11Graphics::GetInstance().GetContext();

	// Context setup - The depth and the blend state should be defined per render section, however we support only opaque geometry without any fancy stuffs

	context.PushPipelineState(DX11PipelineState::kDefault);
	
	// Draws only if there's a camera

	ObjectPtr<ITexture2D> output = nullptr;

	auto main_camera = GetScene().GetMainCamera();

	if (main_camera){

		FrameInfo frame_info;

		frame_info.scene = &GetScene();
		frame_info.camera = main_camera;
		frame_info.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		frame_info.width = width;
		frame_info.height = height;
		frame_info.view_proj_matrix = GetViewProjectionMatrix(frame_info.aspect_ratio);
		frame_info.time_delta = time.GetDeltaSeconds();

		DrawGBuffer(frame_info);							// Scene -> GBuffer
		
		if (enable_global_illumination_) {

			voxelization_->Update(frame_info);				// Dynamic voxelization of the scene

		}

		output = ComputeLighting(frame_info);				// Scene, GBuffer, DepthBuffer -> LightBuffer

	}

	// Cleanup

	context.PopPipelineState();

	immediate_context_->ClearState();
	
	// Done

	graphics_.PopEvent();

	return output;

}

// GBuffer

void DX11DeferredRenderer::DrawGBuffer(const FrameInfo& frame_info){

	graphics_.PushEvent(L"GBuffer");

	// GBuffer initialization

	rt_cache_->PushToCache(ObjectPtr<IRenderTarget>(gbuffer_));			// Release the previous GBuffer, in case the resolution changed

	gbuffer_ = rt_cache_->PopFromCache(frame_info.width,				// Grab a new GBuffer from the cache
									   frame_info.height,
									   { TextureFormat::RGBA_HALF, TextureFormat::RGBA_HALF },
									   true);

	// Bind the GBuffer to the immediate context

	gbuffer_->ClearDepth(*immediate_context_);

	gbuffer_->Bind(*immediate_context_);

	// Draw the visible nodes

	DrawNodes(ComputeVisibleNodes(frame_info.scene->GetMeshHierarchy(), 
								  *frame_info.camera, 
								  frame_info.aspect_ratio), 
			  frame_info);
	
	// Cleanup

	gbuffer_->Unbind(*immediate_context_);

	graphics_.PopEvent();

}

void DX11DeferredRenderer::DrawNodes(const vector<VolumeComponent*>& meshes, const FrameInfo& frame_info){

	graphics_.PushEvent(L"Geometry");

	ObjectPtr<DX11Mesh> mesh;
	ObjectPtr<DX11DeferredRendererMaterial> material;

	// TODO: Implement some batching strategy here!

	for (auto&& node : meshes){

		for (auto&& drawable : node->GetComponents<AspectComponent<DeferredRendererMaterial>>()){

			// Bind the mesh

			mesh = drawable.GetMesh();

			graphics_.PushEvent(mesh->GetName());

			mesh->Bind(*immediate_context_);

			// For each subset

			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index){
				
				graphics_.PushEvent(mesh->GetSubsetName(subset_index));

				// Bind the subset material

				material = drawable.GetMaterial(subset_index);

				material->SetMatrix(drawable.GetWorldTransform(),
									frame_info.view_proj_matrix);

				material->Bind(*immediate_context_);

				// Draw	the subset

				mesh->DrawSubset(*immediate_context_, 
								 subset_index);

				graphics_.PopEvent();

			}

			graphics_.PopEvent();

		}

	}

	graphics_.PopEvent();

}

// Lighting

ObjectPtr<ITexture2D> DX11DeferredRenderer::ComputeLighting(const FrameInfo& frame_info){

	// Accumulate the visible lights

	auto&& visible_lights = ComputeVisibleNodes(frame_info.scene->GetLightHierarchy(), 
												*frame_info.camera, 
												frame_info.aspect_ratio);

	return lighting_->AccumulateLight(ObjectPtr<IRenderTarget>(gbuffer_), 
									  visible_lights,
									  frame_info);

}