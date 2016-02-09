#include "dx11/dx11voxelization.h"

#include "buffer.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11gpgpu.h"
#include "dx11/dx11mesh.h"
#include "dx11/dx11texture.h"
#include "dx11/dx11render_target.h"

#include "dx11/fx/dx11fx_transform.h"

#include "scene.h"
#include "dx11/dx11deferred_renderer.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::windows;
using namespace ::gi_lib::dx11;

namespace {

	/// \brief Vertex shader constant buffer.
	struct VSPerObject {

		Matrix4f world_;							///< \brief World matrix of the object to voxelize.

	};

	struct VSPerFrame{

		Matrix4f view_projection;							///< \brief View-projection matrix.

	};

	struct VoxelParameters {

		Vector3f center_;							// Center of the voxelization.

		float voxel_size_;							// Size of each voxel in world units.

		unsigned int voxel_resolution_;				// Resolution of each cascade in voxels for each dimension

		unsigned int cascades_;						// Number of additional cascades inside the clipmap
		
	};

}

/////////////////////////////////// DX11 VOXELIZATION ///////////////////////////////////

DX11Voxelization::DX11Voxelization(DX11DeferredRenderer& renderer, float voxel_size, unsigned int voxel_resolution, unsigned int cascades) :
	renderer_(renderer){

	bool check;
	
	auto&& app = Application::GetInstance();

	auto&& resources = DX11Resources::GetInstance();

	auto&& device = *DX11Graphics::GetInstance().GetDevice();
	
	// Shared

	voxel_parameters_ = new DX11StructuredBuffer(sizeof(VoxelParameters));

	// Voxel drawing

	static const size_t kDrawIndexedInstancedArguments = 5;

	voxel_draw_indirect_args_ = new DX11GPStructuredArray(DX11GPStructuredArray::CreateDrawIndirectArguments{ kDrawIndexedInstancedArguments });
	
	append_voxel_info_ = resources.Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\append_voxels.hlsl" });

	wireframe_voxel_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\draw_voxel.hlsl" });

	clear_voxel_draw_indirect_args_ = resources.Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\voxel_indirect_args.hlsl" });

	per_frame_ = new DX11StructuredBuffer(sizeof(VSPerFrame));

	scaler_ = make_unique<DX11FxScale>(DX11FxScale::Parameters{});

	check = append_voxel_info_->SetOutput("gIndirectArguments",
										  ObjectPtr<IGPStructuredArray>(voxel_draw_indirect_args_));
	
	check = clear_voxel_draw_indirect_args_->SetOutput("gIndirectArguments",
													   ObjectPtr<IGPStructuredArray>(voxel_draw_indirect_args_));

	check = append_voxel_info_->SetInput("Parameters",
										 ObjectPtr<IStructuredBuffer>(voxel_parameters_));

	check = wireframe_voxel_material_->SetInput("PerFrame",
												ObjectPtr<IStructuredBuffer>(per_frame_));

	render_target_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	BuildVoxelMesh();
	
	// Voxelization

	voxel_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\voxel.hlsl" });

	clear_voxel_address_table_ = resources.Load<IComputation, IComputation::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\common\\clear_uint.hlsl" });

	per_object_ = new DX11StructuredBuffer(sizeof(VSPerObject));

	SetVoxelSize(voxel_size);

	SetVoxelResolution(voxel_resolution,
					   cascades);
	
	check = voxel_material_->SetInput("PerObject",
									  ObjectPtr<IStructuredBuffer>(per_object_));

	check = voxel_material_->SetInput("Parameters",
									  ObjectPtr<IStructuredBuffer>(voxel_parameters_));
	
	// Depth stencil state - No depth test

	D3D11_DEPTH_STENCIL_DESC depth_state_desc;

	ID3D11DepthStencilState* depth_state;
	
	depth_state_desc.DepthEnable = false;
	depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_state_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.StencilEnable = false;
	
	THROW_ON_FAIL(device.CreateDepthStencilState(&depth_state_desc,
												 &depth_state));

	depth_stencil_state_ << &depth_state;
	
	// Rasterizer state - No back-face culling

	D3D11_RASTERIZER_DESC rasterizer_state_desc;

	ID3D11RasterizerState* rasterizer_state;

	rasterizer_state_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_state_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_state_desc.FrontCounterClockwise = false;
	rasterizer_state_desc.DepthBias = 0;
	rasterizer_state_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_state_desc.DepthBiasClamp = 0.0f;
	rasterizer_state_desc.DepthClipEnable = true;
	rasterizer_state_desc.ScissorEnable = false;
	rasterizer_state_desc.MultisampleEnable = false;
	rasterizer_state_desc.AntialiasedLineEnable = false;

	THROW_ON_FAIL(device.CreateRasterizerState(&rasterizer_state_desc,
											   &rasterizer_state));

	rasterizer_state_ << &rasterizer_state;
	
	// Rasterizer state - Wireframe

	rasterizer_state_desc.FillMode = D3D11_FILL_WIREFRAME;
	
	THROW_ON_FAIL(device.CreateRasterizerState(&rasterizer_state_desc,
											   &rasterizer_state));

	wireframe_rasterizer_state_ << &rasterizer_state;

}

void DX11Voxelization::SetVoxelResolution(unsigned int voxel_resolution, unsigned int cascades) {

	voxel_resolution_ = voxel_resolution;
	cascades_ = cascades;

	auto cascade_size = voxel_resolution_ * voxel_resolution_ * voxel_resolution_;												// Amount of voxels in each cascade

	auto pyramid = (1.0f - std::powf(0.125f, std::log2f(static_cast<float>(voxel_resolution_)) + 1.0f)) / (1.0f - 0.125f);		// Size of the clipmap pyramid

	auto voxel_count = cascades_ * cascade_size + pyramid * cascade_size;														// This should be an integer number, otherwise we have a problem!

	// Resize the voxel address table

	voxel_address_table_ = new DX11GPStructuredArray(IGPStructuredArray::FromElementSize{ static_cast<unsigned int>(voxel_count), sizeof(unsigned int)});

	// Resize the debug append buffer

	static const unsigned int kVoxelInfoSize = 12 + 4;		// 3 floats for the center and 1 for the size.

	// Consider that bigger cascades have a greater chance of containing voxels.

	unsigned int voxel_info_count = voxel_count; // voxel_resolution_ * voxel_resolution_ * (1 + cascades_ * 10);		// Random heuristic

	voxel_append_buffer_ = new DX11GPStructuredArray(IGPStructuredArray::CreateAppendBuffer{ voxel_info_count, kVoxelInfoSize });

	// Create the proper render target 

	voxel_render_target_ = new DX11RenderTarget(IRenderTarget::FromDescription{ voxel_resolution, 
																				voxel_resolution, 
																				{ TextureFormat::RGBA_BYTE_UNORM },
																				false});

	bool check;

	check = wireframe_voxel_material_->SetInput("gVoxelAppendBuffer",
												ObjectPtr<IGPStructuredArray>(voxel_append_buffer_));

	check = voxel_material_->SetOutput("gVoxelAddressTable",
									   ObjectPtr<IGPStructuredArray>(voxel_address_table_));

	check = append_voxel_info_->SetInput("gVoxelAddressTable",
										  ObjectPtr<IGPStructuredArray>(voxel_address_table_));

	check = append_voxel_info_->SetOutput("gVoxelAppendBuffer",
										  ObjectPtr<IGPStructuredArray>(voxel_append_buffer_),
										  false);														// Reset the initial count whenever the UAV is bound
	
	check = clear_voxel_address_table_->SetOutput("gBuffer",
												  ObjectPtr<IGPStructuredArray>(voxel_address_table_));

	auto parameters = voxel_parameters_->Lock<VoxelParameters>();
	
	parameters->cascades_ = cascades_;
	parameters->voxel_resolution_ = voxel_resolution_;

	voxel_parameters_->Unlock();

}

void DX11Voxelization::SetVoxelSize(float voxel_size) {

	voxel_size_ = voxel_size;

	voxel_parameters_->Lock<VoxelParameters>()->voxel_size_ = voxel_size_;

	voxel_parameters_->Unlock();

}

float DX11Voxelization::GetGridSize() const {

	float cascade_size = voxel_size_ * voxel_resolution_;			// Size of a single cascade

	return cascade_size * (1 << cascades_);							// Each additional cascade doubles the size of the grid.

}

void DX11Voxelization::BuildVoxelMesh() {

	IStaticMesh::FromVertices<VertexFormatPosition> args;

	// Vertices

	args.vertices.push_back({ Vector3f(-0.5f,  0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f( 0.5f,  0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f( 0.5f,  0.5f,  0.5f) });
	args.vertices.push_back({ Vector3f(-0.5f,  0.5f,  0.5f) });

	args.vertices.push_back({ Vector3f(-0.5f, -0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f( 0.5f, -0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f( 0.5f, -0.5f,  0.5f) });
	args.vertices.push_back({ Vector3f(-0.5f, -0.5f,  0.5f) });

	// Indices

	// Edges only

	args.indices.push_back(0);	args.indices.push_back(1);
	args.indices.push_back(1);	args.indices.push_back(2);
	args.indices.push_back(2);	args.indices.push_back(3);
	args.indices.push_back(3);	args.indices.push_back(0);

	args.indices.push_back(4);	args.indices.push_back(5);
	args.indices.push_back(5);	args.indices.push_back(6);
	args.indices.push_back(6);	args.indices.push_back(7);
	args.indices.push_back(7);	args.indices.push_back(4);

	args.indices.push_back(0);	args.indices.push_back(4);
	args.indices.push_back(1);	args.indices.push_back(5);
	args.indices.push_back(2);	args.indices.push_back(6);
	args.indices.push_back(3);	args.indices.push_back(7);

	// Subset

	args.subsets.push_back({0, args.indices.size()});

	voxel_mesh_ = new DX11Mesh(args);

}

void DX11Voxelization::Update(const FrameInfo& frame_info) {
	
	ObjectPtr<DX11Mesh> mesh;

	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetImmediateContext();

	graphics.PushEvent(L"Dynamic Voxelization");
		
	// Clear the voxel address table first

	graphics.PushEvent(L"Clear");

	clear_voxel_address_table_->Dispatch(device_context,
										 static_cast<unsigned int>(voxel_address_table_->GetCount()),
										 1,
										 1);
	
	graphics.PopEvent();

	// Setup - The material is shared among all the objects
		
	auto& dx_utils = DX11Utils::GetInstance();

	voxel_render_target_->ClearTargets(device_context);
	
	voxel_material_->Bind(device_context, voxel_render_target_);	
	
	dx_utils.PushDepthStencilState(device_context, *depth_stencil_state_);

	dx_utils.PushRasterizerState(device_context, *rasterizer_state_);

	// Voxelize the nodes inside the voxelization domain grid
	
	Vector3f grid_center = frame_info.camera->GetTransformComponent().GetPosition();		// Center of the voxelization, snapped at voxel boundaries to prevent flickering

	grid_center = Vector3f(std::floorf(grid_center(0) / voxel_size_) * voxel_size_,
						   std::floorf(grid_center(1) / voxel_size_) * voxel_size_,
						   std::floorf(grid_center(2) / voxel_size_) * voxel_size_);

	voxel_parameters_->Lock<VoxelParameters>()->center_ = grid_center;

	voxel_parameters_->Unlock();

	AABB grid_domain{ grid_center, 
					  Vector3f::Ones() * GetGridSize() * 0.5f };
	
	vector<VolumeComponent*> nodes = frame_info.scene->GetMeshHierarchy().GetIntersections(grid_domain);
	
	graphics.PushEvent(L"Geometry");

	for (auto&& node : nodes) {

		for (auto&& mesh_component : node->GetComponents<MeshComponent>()) {

			mesh = mesh_component.GetMesh();
			
			graphics.PushEvent(mesh->GetName());

			mesh->Bind(device_context);

			// Constant buffer setup

			per_object_->Lock<VSPerObject>()->world_ = mesh_component.GetWorldTransform().matrix();

			per_object_->Unlock();
			
			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index) {

				// Voxelize the subset

				graphics.PushEvent(mesh->GetSubsetName(subset_index));

				voxel_material_->Commit(device_context);

				mesh->DrawSubset(device_context,
								 subset_index);
				
				graphics.PopEvent();

			}

			graphics.PopEvent();

		}

	}

	graphics.PopEvent();

	// Cleanup

	dx_utils.PopRasterizerState(device_context);
	
	dx_utils.PopDepthStencilState(device_context);
	
	voxel_material_->Unbind(device_context, voxel_render_target_);

	graphics.PopEvent();

}

ObjectPtr<ITexture2D> DX11Voxelization::DrawVoxels(const ObjectPtr<ITexture2D>& image) {
	
	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetImmediateContext();

	auto& dx_utils = DX11Utils::GetInstance();

	if (output_) {

		// Discard the previous image.
		render_target_cache_->PushToCache(ObjectPtr<IRenderTarget>(output_));

	}

	output_ = render_target_cache_->PopFromCache(image->GetWidth(),
												 image->GetHeight(),
												 { image->GetFormat() },
												 false);

	graphics.PushEvent(L"Voxel overlay");

	// Accumulate from the voxel address table inside the append buffer

	graphics.PushEvent(L"Append");

	clear_voxel_draw_indirect_args_->Dispatch(device_context, 1, 1, 1);

	append_voxel_info_->Dispatch(device_context, 
								 static_cast<unsigned int>(voxel_address_table_->GetCount()), 
								 1, 
								 1);
	
	graphics.PopEvent();

	// Dispatch the draw call of the voxelsO

	graphics.PushEvent(L"Draw");

	scaler_->Copy(image,
				  ObjectPtr<IRenderTarget>(output_));

	dx_utils.PushRasterizerState(device_context, *wireframe_rasterizer_state_);

	dx_utils.PushDepthStencilState(device_context, *depth_stencil_state_);

	per_frame_->Lock<VSPerFrame>()->view_projection = renderer_.GetViewProjectionMatrix(static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight()));

	per_frame_->Unlock();

	wireframe_voxel_material_->Bind(device_context);

	output_->Bind(device_context);

	
		
	voxel_mesh_->Bind(device_context);

	device_context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);			// We need only the 12 edges of the cube

	device_context.DrawIndexedInstancedIndirect(voxel_draw_indirect_args_->GetBuffer().Get(),
												0);

	output_->Unbind(device_context);

	wireframe_voxel_material_->Unbind(device_context);

	dx_utils.PopRasterizerState(device_context);

	dx_utils.PopDepthStencilState(device_context);

	graphics.PopEvent();

	// Done

	graphics.PopEvent();
	
	return (*output_)[0];

}

