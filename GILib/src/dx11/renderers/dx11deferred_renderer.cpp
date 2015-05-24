#include "dx11deferred_renderer.h"

#include "..\dx11.h"
#include "..\..\..\include\gimath.h"
#include "..\..\..\include\windows\os_windows.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;
using namespace ::gi_lib::windows;
using namespace ::Eigen;

namespace{

	/// \brief Values that are constants for the entire frame.
	__declspec(align(16))
	struct PerFrameConstants
	{

		Matrix4f world_view_proj_matrix;		///< \brief Projection * View * World matrix.

		Matrix4f world_view_matrix;				///< \brief View * World matrix.

		Matrix4f view_proj_matrix;				///< \brief Projection * View matrix.

		Matrix4f proj_matrix;					///< \brief Projection matrix.

		// Float4

		float near_plane;						///< \brief Near plane distance.

		float far_plane;						///< \brief Far plane distance.

		float z_plane;							///< \brief Unused.

		float w_plane;							///< \brief Unused.

		// Int4

		unsigned int frame_width;				///< \brief Width of the GBuffer in pixels.

		unsigned int frame_height;				///< \brief Height of the GBuffer in pixels.

		unsigned int z_frame;					///< \brief Unused.

		unsigned int w_frame;					///< \brief Unused.

	};

	/// \brief Compute the per-frame constants and fill a target constant buffer.
	/// \param camera The viewer's camera.
	/// \param render_target The render target where the scene will be rendered to.
	/// \param render_context Render context used to map the buffer.
	/// \param constant_buffer Target constant buffer.
	void FillPerFrameConstants(CameraComponent& camera, RenderTarget& render_target, ID3D11DeviceContext& render_context, ID3D11Buffer& constant_buffer){

		D3D11_MAPPED_SUBRESOURCE mapped_buffer;

		// The view matrix is the inverse world matrix of the camera.
		Matrix4f camera_view = camera.GetComponent<TransformComponent>()->GetWorldTransform().matrix().inverse();

		Matrix4f camera_projection = Matrix4f::Identity();
		
		Matrix4f camera_view_projection = camera_projection * camera_view;

		Matrix4f world_matrix = Matrix4f::Identity();

		auto world_view_proj_matrix = world_matrix * camera_view_projection;

		// Compute composite matrices
				
		render_context.Map(&constant_buffer,			// Target constant buffer
						   0,							// First subresource
						   D3D11_MAP_WRITE_DISCARD,		// Discard the previous buffer
						   0,							// Flags
						   &mapped_buffer);				

		PerFrameConstants& buffer = *static_cast<PerFrameConstants *>(mapped_buffer.pData);

		// Matrices

		buffer.world_view_proj_matrix = world_view_proj_matrix;
		buffer.world_view_matrix = camera_view * world_matrix;
		buffer.view_proj_matrix = camera_view_projection;
		buffer.proj_matrix = camera_projection;

		// Camera planes

		buffer.near_plane = camera.GetMinimumDistance();
		buffer.far_plane = camera.GetMaximumDistance();
		buffer.z_plane = 0.0f;
		buffer.w_plane = 0.0f;

		// Frame dimensions
		
		buffer.frame_width = render_target.GetTexture(0)->GetWidth();
		buffer.frame_height = render_target.GetTexture(0)->GetHeight();
		buffer.z_frame = 0;
		buffer.w_frame = 0;
		
		// End of mapping

		render_context.Unmap(&constant_buffer, 
							 0);
	
	}

	void DrawIndexedSubset(ID3D11DeviceContext& context, const MeshSubset& subset){

		context.DrawIndexed(static_cast<unsigned int>(subset.count),
							static_cast<unsigned int>(subset.start_index),
							0);

	}

}

///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const CompileFromFile& args) :
material_(new DX11Material(args))
{}

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const Instantiate& args) :
material_(new DX11Material(Material::Instantiate{ args.base->GetMaterial() })){}

///////////////////////////////// DX11 TILED DEFERRED RENDERER //////////////////////////////////

DX11TiledDeferredRenderer::DX11TiledDeferredRenderer(const RendererConstructionArgs& arguments) :
TiledDeferredRenderer(arguments.scene){

	auto& device = DX11Graphics::GetInstance().GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ = std::move(unique_com(context));

	// Create the per-frame constants const buffer
	ID3D11Buffer* per_frame_constants;

	MakeConstantBuffer(device, 
					   sizeof(PerFrameConstants), 
					   &per_frame_constants);

	per_frame_constants_ = std::move(unique_com(per_frame_constants));
	
}

DX11TiledDeferredRenderer::~DX11TiledDeferredRenderer(){}

void DX11TiledDeferredRenderer::Draw(IOutput& output){
	
	// Scene to draw
	auto& scene = GetScene();

	// The cast is safe as long as the client is not mixing different APIs.
	auto& dx11output = static_cast<DX11Output&>(output);

	// Draws only if there's a camera

	if (scene.GetMainCamera()){

		auto& camera = *scene.GetMainCamera();

		auto render_target = resource_cast(dx11output.GetRenderTarget());
	
		// Frustum culling

		auto nodes = scene.GetVolumeHierarchy()
						  .GetIntersections(camera.GetViewFrustum(render_target->GetAspectRatio()),		// Updates the view frustum according to the output ratio.
										    IVolumeHierarchy::PrecisionLevel::Medium);					// Avoids extreme false positive while keeping reasonably high performances.

		// Compute frame constants

		FillPerFrameConstants(camera,
							  *render_target,
							  *immediate_context_,
							  *per_frame_constants_);

		// Setup of the render context

		immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		render_target->ClearDepthStencil(*immediate_context_,
										 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
										 1.0f,
										 0);

		Color color;

		color.color.alpha = 0.0f;
		color.color.red = 0.0f;
		color.color.green = 0.5f;
		color.color.blue = 0.5f;

		render_target->ClearTargets(*immediate_context_, 
									color);

		// Bind the render target

		render_target->Bind(*immediate_context_);

		// Draw GBuffer
		for (auto&& node : nodes){

			// Items to draw

			for (auto&& drawable : node->GetComponents<DeferredRendererComponent>()){

				// Bind the mesh

				auto mesh = resource_cast(drawable.GetMesh());

				mesh->Bind(*immediate_context_);

				// For each subset
				for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index){

					// Bind the material
					
					auto deferred_material = drawable.GetMaterial(subset_index);

					auto material = resource_cast(deferred_material->GetMaterial());

					material->Commit(*immediate_context_);

					// Draw	the subset
					DrawIndexedSubset(*immediate_context_,
									  mesh->GetSubset(subset_index));

				}

			}

		}

		// Compute lighting

	}

	// Restore the rendering context

	immediate_context_->ClearState();

	// Present the image
	dx11output.Present();

}

