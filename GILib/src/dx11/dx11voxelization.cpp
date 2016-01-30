#include "dx11/dx11voxelization.h"

#include "buffer.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11gpgpu.h"
#include "dx11/dx11mesh.h"
#include "scene.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::windows;
using namespace ::gi_lib::dx11;

namespace {

	/// \brief Vertex shader constant buffer.
	struct VSPerObject {

		Matrix4f world_;							///< \brief World matrix of the object to voxelize.

	};

	struct VoxelParameters {

		Vector3f center_;							// Center of the voxelization.

		float voxel_size_;							// Size of each voxel in world units.

		unsigned int voxel_resolution_;				// Resolution of each cascade in voxels for each dimension

		unsigned int cascades_;						// Number of additional cascades inside the clipmap
		
	};

}

/////////////////////////////////// DX11 VOXELIZATION ///////////////////////////////////

DX11Voxelization::DX11Voxelization(float voxel_size, unsigned int voxel_resolution, unsigned int cascades) {

	voxel_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\voxel.hlsl" });

	per_object_ = new DX11StructuredBuffer(sizeof(VSPerObject));

	voxel_parameters_ = new DX11StructuredBuffer(sizeof(VoxelParameters));

	SetVoxelSize(voxel_size);

	SetVoxelResolution(voxel_resolution,
					   cascades);
	
	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	// Create the depth stencil state - No depth test

	D3D11_DEPTH_STENCIL_DESC depth_state_desc;

	ID3D11DepthStencilState* depth_state;
	
	depth_state_desc.DepthEnable = false;
	depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_state_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.StencilEnable = false;
	
	THROW_ON_FAIL(device.CreateDepthStencilState(&depth_state_desc,
												 &depth_state));

	depth_stencil_state_ << &depth_state;
	
	// Create the raster state - No back-face culling

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

	// One-time setup
	
	bool check;

	check = voxel_material_->SetInput("PerObject",
									  ObjectPtr<IStructuredBuffer>(per_object_));

	check = voxel_material_->SetInput("Parameters",
									  ObjectPtr<IStructuredBuffer>(voxel_parameters_));

}

void DX11Voxelization::SetVoxelResolution(unsigned int voxel_resolution, unsigned int cascades) {

	voxel_resolution_ = voxel_resolution;
	cascades_ = cascades;

	auto cascade_size = voxel_resolution_ * voxel_resolution_ * voxel_resolution_;												// Amount of voxels in each cascade

	auto pyramid = (1.0f - std::powf(0.125f, std::log2f(static_cast<float>(voxel_resolution_)) + 1.0f)) / (1.0f - 0.125f);		// Size of the clipmap pyramid

	auto voxel_count = cascades_ * cascade_size + pyramid * cascade_size;														// This should be an integer number, otherwise we have a problem!

	// Resize the voxel address table

	voxel_address_table_ = new DX11GPStructuredArray(IGPStructuredArray::FromElementSize{ static_cast<unsigned int>(voxel_count), sizeof(unsigned int)});

	// Create the proper render target 

	voxel_render_target_ = new DX11RenderTarget(IRenderTarget::FromDescription{ voxel_resolution, 
																				voxel_resolution, 
																				{ TextureFormat::RGBA_BYTE_UNORM },
																				false});

	bool check;

	check = voxel_material_->SetOutput("gVoxelAddressTable",
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

void DX11Voxelization::Update(const FrameInfo& frame_info) {
	
	ObjectPtr<DX11Mesh> mesh;

	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetImmediateContext();

	graphics.PushEvent(L"Dynamic Voxelization");

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