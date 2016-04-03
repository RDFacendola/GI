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

#include "wavefront/wavefront_obj.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::windows;
using namespace ::gi_lib::dx11;

namespace {

	/// \brief Info about a single voxel.
	/// \see See voxel_def.hlsl
	struct VoxelInfo {	

		Vector3f center;					// Center of the voxel, in world space

		float size;							// Size of the voxel in world units

		Vector4f red_sh01;					// First and second SH coefficients for the red channel.

		Vector4f green_sh01;				// First and second SH coefficients for the green channel.

		Vector4f blue_sh01;					// First and second SH coefficients for the blue channel.

	};

	/// \brief Constant buffer containing the per-object constants.
	struct CBObject {

		Matrix4f world_;							///< \brief World matrix of the object to voxelize.

	};

	/// \brief Constant buffer containing the per-frame constants.
	struct CBFrame{

		Matrix4f view_projection;							///< \brief View-projection matrix.

	};

	/// \brief Constant buffer containing voxelization parameters
	struct CBVoxelization {

		Vector3f center_;							// Center of the voxelization.

		float voxel_size_;							// Size of each voxel in world units.

		unsigned int voxel_resolution_;				// Resolution of each cascade in voxels for each dimension

		unsigned int cascades_;						// Number of additional cascades inside the clipmap
		
	};

}

/////////////////////////////////// DX11 VOXELIZATION :: DEBUG DRAWER ///////////////////////////////////

class DX11Voxelization::DebugDrawer {

public:

	static const Tag kVoxelAppendBuffer;			///< \brief Tag associated to the append buffer used while gathering voxels for debug draw.

	static const Tag kVoxelDrawIndirectArgs;		///< \brief Tag associated to the buffer containing the arguments used to draw the voxels.

	static const Tag kSHDrawIndirectArgs;			///< \brief Tag associated to the buffer containing the arguments used to draw the spherical harmonics.

	static const Tag kPerFrameTag;					///< \brief Tag associated to the buffer containing the arguments used to draw the voxels.

	DebugDrawer(DX11Voxelization& voxelization);

	ObjectPtr<ITexture2D> DrawDebugOverlay(const ObjectPtr<ITexture2D>& image, const Matrix4f& view_projection) const;

private:

	/// \brief Draw the voxel outline on top of the provided output surface.
	/// \param output Surface where the voxel should be drawn onto.
	void DrawVoxels(const ObjectPtr<DX11RenderTarget>& output) const;

	/// \brief Draw the spherical harmonics outline on top of the provided output surface.
	/// \param output Surface where the spherical harmonics should be drawn onto.
	void DrawSphericalHarmonics(const ObjectPtr<DX11RenderTarget>& output) const;

	/// \brief Draw the voxel depth on top of the provided output surface.
	/// \param pipeline_state Pipeline state to use while drawing the voxel depth.
	void DrawVoxelDepth(const ObjectPtr<DX11RenderTarget>& output, const DX11PipelineState& pipeline_state) const;

	/// \brief Create the debug meshes.
	void BuildMeshes();

	/// \brief Initialize the shader resources.
	void InitResources();

	/// \brief Initialize the shaders.
	void InitShaders();

	DX11Voxelization& subject_;											///< \brief Parent class holding voxelization structures.

	DX11Graphics& graphics_;											///< \brief Graphics instance.
	
	DX11Context& context_;												///< \brief Graphics context.

	// Shader resources

	mutable ObjectPtr<DX11RenderTarget> output_;						///< \brief Surface where the debug infos are being rendered to.

	ObjectPtr<IRenderTargetCache> render_target_cache_;					///< \brief Cache of render-target textures.

	ObjectPtr<DX11GPStructuredArray> voxel_draw_indirect_args_;			///< \brief Buffer containing the argument buffer used to dispatch the DrawIndexedInstancedIndirect call

	ObjectPtr<DX11GPStructuredArray> sh_draw_indirect_args_;			///< \brief Buffer containing the argument buffer used to dispatch the DrawInstancedIndirect call

	ObjectPtr<DX11GPStructuredArray> voxel_append_buffer_;				///< \brief Append buffer containing the debug voxel info (namely their center and their size).

	ObjectPtr<DX11StructuredBuffer> cb_frame_;							///< \brief Per-frame constant buffer used during voxel draw.

	DX11PipelineState voxel_prepass_state_;								///< \brief Pipeline state for voxel prepass stage.

	DX11PipelineState sh_prepass_state_;								///< \brief Pipeline state for spherical harmonic prepass stage.
	
	DX11PipelineState sh_alphablend_state_;								///< \brief Pipeline state for spherical harmonic debug draw.

	// Shader

	ObjectPtr<DX11Material> voxel_depth_only_material_;					///< \brief Used to draw instanced voxels on the depth buffer.

	ObjectPtr<DX11Material> voxel_outline_material_;					///< \brief Material used to draw instanced voxels outline.

	ObjectPtr<DX11Material> sh_outline_material_;						///< \brief Material used to draw instanced spherical harmonics.
	
	ObjectPtr<DX11Computation> clear_voxel_draw_indirect_args_;			///< \brief Compute shader used to clear the voxel draw indirect arguments.

	ObjectPtr<DX11Computation> append_voxel_info_;						///< \brief Compute shader used to append voxel info inside the voxel append buffer.
	
	std::unique_ptr<DX11FxScale> scaler_;								///< \brief Used to copy the input image.

	// Meshes

	ObjectPtr<DX11Mesh> cube_edges_mesh_;								///< \brief Edges of a cube, used to outline a voxel.

	ObjectPtr<DX11Mesh> cube_mesh_;										///< \brief Solid cube used for debug voxel prepass.

	ObjectPtr<DX11Mesh> sphere_mesh_;									///< \brief Sphere mesh used to draw spherical harmonics

};

const Tag DX11Voxelization::DebugDrawer::kVoxelAppendBuffer= "gVoxelAppendBuffer";
const Tag DX11Voxelization::DebugDrawer::kVoxelDrawIndirectArgs = "gVoxelIndirectArguments";
const Tag DX11Voxelization::DebugDrawer::kSHDrawIndirectArgs = "gSHIndirectArguments";
const Tag DX11Voxelization::DebugDrawer::kPerFrameTag = "PerFrame";

DX11Voxelization::DebugDrawer::DebugDrawer(DX11Voxelization& voxelization) :
subject_(voxelization),
graphics_(DX11Graphics::GetInstance()),
context_(graphics_.GetContext()){
	
	InitResources();
	InitShaders();	

}

void DX11Voxelization::DebugDrawer::InitResources() {

	auto& resources = DX11Resources::GetInstance();

	voxel_draw_indirect_args_ = new DX11GPStructuredArray(DX11GPStructuredArray::CreateDrawIndirectArguments{ 5 });
	
	sh_draw_indirect_args_ = new DX11GPStructuredArray(DX11GPStructuredArray::CreateDrawIndirectArguments{ 4 });

	voxel_append_buffer_ = new DX11GPStructuredArray(IGPStructuredArray::CreateAppendBuffer{ std::powf(subject_.GetVoxelCount(), 0.85f), sizeof(VoxelInfo) });		// 0.85f is just an heuristic...

	cb_frame_ = new DX11StructuredBuffer(sizeof(CBFrame));
	
	render_target_cache_ = resources.Load<IRenderTargetCache, IRenderTargetCache::Singleton>({});

	// Pipeline states

	sh_prepass_state_.SetWriteMode(false, true, D3D11_COMPARISON_LESS)			// Disable color write.
					 .SetRasterMode(D3D11_FILL_SOLID, D3D11_CULL_FRONT);		// Reverse culling

	voxel_prepass_state_.SetWriteMode(false, true, D3D11_COMPARISON_LESS)		// Disable color write.
						.SetDepthBias(100, 0.0f, 0.0f);							// Depth is biased to reduce artifact while drawing voxel edges.

	sh_alphablend_state_.EnableAlphaBlend(true);								// Alphablend enable.

	// Meshes

	BuildMeshes();

}

void DX11Voxelization::DebugDrawer::InitShaders() {

	auto& resources = DX11Resources::GetInstance();

	// Shader loading

	append_voxel_info_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\append_voxels.hlsl" });
	
	clear_voxel_draw_indirect_args_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\voxel_indirect_args.hlsl" });
		
	voxel_outline_material_ = new DX11Material(IMaterial::CompileFromFile{ L"Data\\Shaders\\voxel\\voxel_outline.hlsl" });

	voxel_depth_only_material_ = new DX11Material(IMaterial::CompileFromFile{ L"Data\\Shaders\\voxel\\voxel_depth_only.hlsl" });

	sh_outline_material_ = new DX11Material(IMaterial::CompileFromFile{ L"Data\\Shaders\\voxel\\sh_outline.hlsl" });

	scaler_ = make_unique<DX11FxScale>(DX11FxScale::Parameters{});
	
	// Shader setup

	bool check;
	
	check = append_voxel_info_->SetInput(DX11Voxelization::kVoxelAddressTableTag,
										 ObjectPtr<IGPStructuredArray>(subject_.voxel_address_table_));

	check = append_voxel_info_->SetOutput(DebugDrawer::kVoxelAppendBuffer,
										  ObjectPtr<IGPStructuredArray>(voxel_append_buffer_),
										  false);														// Reset the initial count whenever the UAV is bound

	check = append_voxel_info_->SetOutput(kVoxelDrawIndirectArgs,
										  ObjectPtr<IGPStructuredArray>(voxel_draw_indirect_args_));

	check = append_voxel_info_->SetOutput(kSHDrawIndirectArgs,
										  ObjectPtr<IGPStructuredArray>(sh_draw_indirect_args_));

	check = append_voxel_info_->SetInput(DX11Voxelization::kVoxelizationTag,
										 ObjectPtr<IStructuredBuffer>(subject_.cb_voxelization_));

	check = append_voxel_info_->SetInput(DX11Voxelization::kRedSH01Tag, 
										 subject_.GetSH(0)->GetTexture());

	check = append_voxel_info_->SetInput(DX11Voxelization::kGreenSH01Tag, 
										 subject_.GetSH(1)->GetTexture());

	check = append_voxel_info_->SetInput(DX11Voxelization::kBlueSH01Tag, 
										 subject_.GetSH(2)->GetTexture());

	//

	check = clear_voxel_draw_indirect_args_->SetOutput(kVoxelDrawIndirectArgs,
													   ObjectPtr<IGPStructuredArray>(voxel_draw_indirect_args_));
	
	check = clear_voxel_draw_indirect_args_->SetOutput(kSHDrawIndirectArgs,
													   ObjectPtr<IGPStructuredArray>(sh_draw_indirect_args_));
	//

	check = voxel_depth_only_material_->SetInput(DebugDrawer::kVoxelAppendBuffer,
												 ObjectPtr<IGPStructuredArray>(voxel_append_buffer_));
	
	check = voxel_depth_only_material_->SetInput(DebugDrawer::kPerFrameTag,
												 ObjectPtr<IStructuredBuffer>(cb_frame_));
	
	check = voxel_depth_only_material_->SetInput(DX11Voxelization::kVoxelizationTag,
												 ObjectPtr<IStructuredBuffer>(subject_.cb_voxelization_));
	
	//

	check = voxel_outline_material_->SetInput(DebugDrawer::kVoxelAppendBuffer,
											  ObjectPtr<IGPStructuredArray>(voxel_append_buffer_));
	
	check = voxel_outline_material_->SetInput(DebugDrawer::kPerFrameTag,
											  ObjectPtr<IStructuredBuffer>(cb_frame_));
	
	check = voxel_outline_material_->SetInput(DX11Voxelization::kVoxelizationTag,
										 ObjectPtr<IStructuredBuffer>(subject_.cb_voxelization_));

	//

	check = sh_outline_material_->SetInput(DebugDrawer::kVoxelAppendBuffer,
										   ObjectPtr<IGPStructuredArray>(voxel_append_buffer_));
	
	check = sh_outline_material_->SetInput(DebugDrawer::kPerFrameTag,
										   ObjectPtr<IStructuredBuffer>(cb_frame_));
	
	check = sh_outline_material_->SetInput(DX11Voxelization::kVoxelizationTag,
										   ObjectPtr<IStructuredBuffer>(subject_.cb_voxelization_));

}

ObjectPtr<ITexture2D> DX11Voxelization::DebugDrawer::DrawDebugOverlay(const ObjectPtr<ITexture2D>& image, const Matrix4f& view_projection) const {

	if (output_) {

		// Discard the previous image.
		render_target_cache_->PushToCache(ObjectPtr<IRenderTarget>(output_));

	}

	output_ = render_target_cache_->PopFromCache(image->GetWidth(),
												 image->GetHeight(),
												 { image->GetFormat() },
												 true);

	auto& device_context = *context_.GetImmediateContext();

	graphics_.PushEvent(L"Voxel overlay");

	// Accumulate from the voxel address table inside the append buffer

	graphics_.PushEvent(L"Setup");

	clear_voxel_draw_indirect_args_->Dispatch(device_context, 
											  1, 
											  1,
											  1);

	append_voxel_info_->Dispatch(device_context, 
								 static_cast<unsigned int>(subject_.voxel_address_table_->GetCount()), 
								 1, 
								 1);
	
	scaler_->Copy(image,
				  ObjectPtr<IRenderTarget>(output_));
	
	graphics_.PopEvent();	// Setup

	// Dispatch the draw call of the voxels

	cb_frame_->Lock<CBFrame>()->view_projection = view_projection;

	cb_frame_->Unlock();
		
	output_->Bind(device_context);

	DrawSphericalHarmonics(output_);

	DrawVoxels(output_);

	output_->Unbind(device_context);

	// Done
	
	graphics_.PopEvent();	// Voxel overlay
		
	return (*output_)[0];

}

void DX11Voxelization::DebugDrawer::DrawVoxels(const ObjectPtr<DX11RenderTarget>& output) const{

	auto& context = graphics_.GetContext();

	auto& device_context = *context.GetImmediateContext();

	graphics_.PushEvent(L"Voxels");

	DrawVoxelDepth(output, voxel_prepass_state_);

	// Draw - Draw the actual voxels

	context.PushPipelineState(DX11PipelineState::kDefault);

	voxel_outline_material_->Bind(device_context);

	cube_edges_mesh_->Bind(device_context);

	device_context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);			// Override primitive type

	device_context.DrawIndexedInstancedIndirect(voxel_draw_indirect_args_->GetBuffer().Get(),
												0);

	voxel_outline_material_->Unbind(device_context);

	context.PopPipelineState();
	
	graphics_.PopEvent();

}

void DX11Voxelization::DebugDrawer::DrawSphericalHarmonics(const ObjectPtr<DX11RenderTarget>& output) const{

	auto& context = graphics_.GetContext();

	auto& device_context = *context.GetImmediateContext();

	graphics_.PushEvent(L"Spherical harmonics");

	DrawVoxelDepth(output, sh_prepass_state_);

	// Draw - Draw the actual spherical harmonics

	context.PushPipelineState(sh_alphablend_state_);

	sh_outline_material_->Bind(device_context);

	sphere_mesh_->Bind(device_context);

	device_context.DrawInstancedIndirect(sh_draw_indirect_args_->GetBuffer().Get(),
										 0);

	sh_outline_material_->Unbind(device_context);

	context.PopPipelineState();

	graphics_.PopEvent();

}

void DX11Voxelization::DebugDrawer::DrawVoxelDepth(const ObjectPtr<DX11RenderTarget>& output, const DX11PipelineState& pipeline_state) const {

	auto& device_context = *context_.GetImmediateContext();

	output->ClearDepth(device_context);

	context_.PushPipelineState(pipeline_state);				// Pipeline state

	voxel_depth_only_material_->Bind(device_context);		// Material

	cube_mesh_->Bind(device_context);						// Mesh

	device_context.DrawIndexedInstancedIndirect(voxel_draw_indirect_args_->GetBuffer().Get(),
												0);

	voxel_depth_only_material_->Unbind(device_context);

	context_.PopPipelineState();

}

void DX11Voxelization::DebugDrawer::BuildMeshes() {

	IStaticMesh::FromVertices<VertexFormatPosition> args;

	// Vertices - Shared for both the edge version and the cube one

	args.vertices.push_back({ Vector3f(-0.5f,  0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f(0.5f,  0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f(0.5f,  0.5f,  0.5f) });
	args.vertices.push_back({ Vector3f(-0.5f,  0.5f,  0.5f) });

	args.vertices.push_back({ Vector3f(-0.5f, -0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f(0.5f, -0.5f, -0.5f) });
	args.vertices.push_back({ Vector3f(0.5f, -0.5f,  0.5f) });
	args.vertices.push_back({ Vector3f(-0.5f, -0.5f,  0.5f) });

	// Indices - Edges only

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

	// Subset - Edges only

	args.subsets.push_back({ 0, args.indices.size() });

	cube_edges_mesh_ = new DX11Mesh(args);

	// Indices - Cube faces

	args.indices.clear();

	args.indices.push_back(3);    args.indices.push_back(1);    args.indices.push_back(0);
	args.indices.push_back(2);    args.indices.push_back(1);    args.indices.push_back(3);
	args.indices.push_back(6);    args.indices.push_back(4);    args.indices.push_back(5);
	args.indices.push_back(7);    args.indices.push_back(4);    args.indices.push_back(6);
	args.indices.push_back(3);    args.indices.push_back(4);    args.indices.push_back(7);
	args.indices.push_back(0);    args.indices.push_back(4);    args.indices.push_back(3);
	args.indices.push_back(1);    args.indices.push_back(6);    args.indices.push_back(5);
	args.indices.push_back(2);    args.indices.push_back(6);    args.indices.push_back(1);
	args.indices.push_back(0);    args.indices.push_back(5);    args.indices.push_back(4);
	args.indices.push_back(1);    args.indices.push_back(5);    args.indices.push_back(0);
	args.indices.push_back(2);    args.indices.push_back(7);    args.indices.push_back(6);
	args.indices.push_back(3);    args.indices.push_back(7);    args.indices.push_back(2);

	// Subset - Cube faces

	args.subsets.push_back({ 0, args.indices.size() });

	cube_mesh_ = new DX11Mesh(args);

	// Sphere - Load from file

	auto& resources = DX11Resources::GetInstance();

	wavefront::ObjImporter obj_importer(resources);

	sphere_mesh_ = obj_importer.ImportMesh(L"Data\\assets\\Light\\Sphere.obj", "Icosphere");

}

/////////////////////////////////// DX11 VOXELIZATION ///////////////////////////////////

const Tag DX11Voxelization::kVoxelAddressTableTag = "gVoxelAddressTable";
const Tag DX11Voxelization::kVoxelizationTag = "Voxelization";
const Tag DX11Voxelization::kRedSH01Tag = "gRSH01";
const Tag DX11Voxelization::kGreenSH01Tag = "gGSH01";
const Tag DX11Voxelization::kBlueSH01Tag = "gBSH01";

DX11Voxelization::DX11Voxelization(DX11DeferredRenderer& renderer, float voxel_size, unsigned int voxel_resolution, unsigned int cascades) :
voxel_size_(voxel_size),
voxel_resolution_(voxel_resolution),
cascades_(cascades),
renderer_(renderer){
	
	InitResources();
	InitShaders();

	debug_drawer_ = std::make_unique<DebugDrawer>(*this);

}

void DX11Voxelization::InitResources() {

	cb_voxelization_ = new DX11StructuredBuffer(sizeof(CBVoxelization));

	cb_object_ = new DX11StructuredBuffer(sizeof(CBObject));

	voxel_address_table_ = new DX11GPStructuredArray(IGPStructuredArray::FromElementSize{ GetVoxelCount(), sizeof(unsigned int)});

	for (size_t channel_index = 0; channel_index < 3; ++channel_index) {

		voxel_sh_01_[channel_index] = new DX11GPTexture3D(IGPTexture3D::FromDescription{ voxel_resolution_,
																						 voxel_resolution_,
																						 voxel_resolution_ * (1 + cascades_),
																						 1,
																						 TextureFormat::RGBA_HALF });

	}
	
	voxel_render_target_ = new DX11RenderTarget(IRenderTarget::FromDescription{ voxel_resolution_, 
																				voxel_resolution_, 
																				{}, 
																				false });	// Empty render target view

	voxelization_state_.SetWriteMode(false, false, D3D11_COMPARISON_ALWAYS)		// Disable both color and depth write.
					   .SetRasterMode(D3D11_FILL_SOLID, D3D11_CULL_NONE);		// Disable culling

	// Constants

	auto parameters = cb_voxelization_->Lock<CBVoxelization>();

	parameters->voxel_size_ = voxel_size_;
	parameters->cascades_ = cascades_;
	parameters->voxel_resolution_ = voxel_resolution_;

	cb_voxelization_->Unlock();

}

void DX11Voxelization::InitShaders() {

	auto& resources = DX11Resources::GetInstance();

	// Shader loading

	voxel_material_ = new DX11Material(IMaterial::CompileFromFile{ L"Data\\Shaders\\voxel\\voxel.hlsl" });

	clear_voxel_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\voxel_clear.hlsl" });

	clear_sh_ = resources.Load<IComputation, IComputation::CompileFromFile>({ L"Data\\Shaders\\voxel\\sh_clear.hlsl" });

	// Shader setup

	bool check;

	check = voxel_material_->SetOutput(kVoxelAddressTableTag,
									   ObjectPtr<IGPStructuredArray>(voxel_address_table_));
	
	check = voxel_material_->SetInput("PerObject",
									  ObjectPtr<IStructuredBuffer>(cb_object_));

	check = voxel_material_->SetInput(kVoxelizationTag,
									  ObjectPtr<IStructuredBuffer>(cb_voxelization_));
		

	check = clear_voxel_->SetOutput(kVoxelAddressTableTag,
									ObjectPtr<IGPStructuredArray>(voxel_address_table_));

	check = clear_sh_->SetOutput(DX11Voxelization::kRedSH01Tag,
								 ObjectPtr<IGPTexture3D>(voxel_sh_01_[0]));

	check = clear_sh_->SetOutput(DX11Voxelization::kGreenSH01Tag,
								 ObjectPtr<IGPTexture3D>(voxel_sh_01_[1]));

	check = clear_sh_->SetOutput(DX11Voxelization::kBlueSH01Tag,
								 ObjectPtr<IGPTexture3D>(voxel_sh_01_[2]));

}

DX11Voxelization::~DX11Voxelization() {

}

float DX11Voxelization::GetGridSize() const {

	float cascade_size = voxel_size_ * voxel_resolution_;			// Size of a single cascade

	return cascade_size * (1 << cascades_);							// Each additional cascade doubles the size of the grid.

}

void DX11Voxelization::Update(const FrameInfo& frame_info) {
	
	ObjectPtr<DX11Mesh> mesh;

	auto& graphics = DX11Graphics::GetInstance();

	auto& context = graphics.GetContext();

	auto& device_context = *context.GetImmediateContext();

	graphics.PushEvent(L"Dynamic Voxelization");

	Clear();

	// Setup - The material is shared among all the objects

	voxel_render_target_->ClearTargets(device_context);
	
	voxel_material_->Bind(device_context, voxel_render_target_);	
	
	context.PushPipelineState(voxelization_state_);

	// Voxelize the nodes inside the voxelization domain grid
	
	Vector3f grid_center = frame_info.camera->GetTransformComponent().GetPosition();		// Center of the voxelization, snapped at voxel boundaries to prevent flickering

	float snap = voxel_size_ / (1 << cascades_);

	grid_center = Vector3f(std::floorf(grid_center(0) / snap) * snap,
						   std::floorf(grid_center(1) / snap) * snap,
						   std::floorf(grid_center(2) / snap) * snap);

	cb_voxelization_->Lock<CBVoxelization>()->center_ = grid_center;

	cb_voxelization_->Unlock();

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

			cb_object_->Lock<CBObject>()->world_ = mesh_component.GetWorldTransform().matrix();

			cb_object_->Unlock();
			
			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index) {

				// Voxelize the subset

				graphics.PushEvent(mesh->GetSubsetName(subset_index));

				voxel_material_->Commit(device_context);

				mesh->DrawSubset(device_context,
								 subset_index,
								 cascades_ + 1);
				
				graphics.PopEvent();

			}

			graphics.PopEvent();

		}

	}

	graphics.PopEvent();

	// Cleanup

	context.PopPipelineState();

	voxel_material_->Unbind(device_context, voxel_render_target_);

	graphics.PopEvent();

}

void DX11Voxelization::Clear() {

	auto& graphics = DX11Graphics::GetInstance();

	auto& device_context = *graphics.GetContext().GetImmediateContext();

	// Clear the voxel address table and the spherical harmonics

	graphics.PushEvent(L"Clear");

	clear_voxel_->Dispatch(device_context,
						   static_cast<unsigned int>(voxel_address_table_->GetCount()),
						   1,
						   1);

	clear_sh_->Dispatch(device_context,
						static_cast<unsigned int>(voxel_sh_01_[0]->GetWidth()),
						static_cast<unsigned int>(voxel_sh_01_[0]->GetHeight()),
						static_cast<unsigned int>(voxel_sh_01_[0]->GetDepth()));

	graphics.PopEvent();

}

ObjectPtr<ITexture2D> DX11Voxelization::DrawVoxels(const ObjectPtr<ITexture2D>& image) {
	
	return debug_drawer_->DrawDebugOverlay(image,
										   renderer_.GetViewProjectionMatrix(static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight())));

}

unsigned int DX11Voxelization::GetVoxelCount() const {

	auto cascade_size = voxel_resolution_ * voxel_resolution_ * voxel_resolution_;		// Amount of voxels in each cascade

	auto pyramid_size = static_cast<unsigned int>(Math::SumGeometricSeries(cascade_size, 
																		   0.125f, 
																		   std::log2f(static_cast<float>(voxel_resolution_)) + 1.f));

	return cascades_ * cascade_size + pyramid_size;

}
