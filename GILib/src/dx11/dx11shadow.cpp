#include "dx11/dx11shadow.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11mesh.h"

#include "core.h"
#include "light_component.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;

namespace {

	/// \brief Draw a mesh subset using the given context.
	inline void DrawIndexedSubset(ID3D11DeviceContext& context, const MeshSubset& subset) {

		context.DrawIndexed(static_cast<unsigned int>(subset.count),
							static_cast<unsigned int>(subset.start_index),
							0);

	}

}

///////////////////////////////////// DX11 VSM ATLAS /////////////////////////////////////////

const Tag DX11VSMAtlas::kPerObject = "PerObject";
const Tag DX11VSMAtlas::kPerLight = "PerLight";

DX11VSMAtlas::DX11VSMAtlas(unsigned int width, unsigned height, unsigned int pages, bool full_precision) {

	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ << &context;

	// 

	D3D11_RASTERIZER_DESC rasterizer_state_desc;

	ID3D11RasterizerState* rasterizer_state;

	rasterizer_state_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_state_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_state_desc.FrontCounterClockwise = false;
	rasterizer_state_desc.DepthBias = 10000;
	rasterizer_state_desc.SlopeScaledDepthBias = 0.1f;
	rasterizer_state_desc.DepthBiasClamp = 0.0f;
	rasterizer_state_desc.DepthClipEnable = true;
	rasterizer_state_desc.ScissorEnable = false;
	rasterizer_state_desc.MultisampleEnable = false;
	rasterizer_state_desc.AntialiasedLineEnable = false;

	device.CreateRasterizerState(&rasterizer_state_desc,
								 &rasterizer_state);

	rasterizer_state_ << &rasterizer_state;

	// Create the shadow resources

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, 8 });

	atlas_ = new DX11RenderTargetArray(width, 
									   height, 
									   pages, 
									   full_precision ? DXGI_FORMAT_R32G32_FLOAT : DXGI_FORMAT_R16G16_FLOAT);
	
	shadow_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\vsm.hlsl" });

	per_object_ = new DX11StructuredBuffer(sizeof(PerObject));

	per_light_ = new DX11StructuredBuffer(sizeof(PerLight));

	// One-time setup

	bool check;

	check = shadow_material_->SetInput(kPerObject,
									   ObjectPtr<IStructuredBuffer>(per_object_));

	check = shadow_material_->SetInput(kPerLight,
									   ObjectPtr<IStructuredBuffer>(per_light_));

	point_shadows_ = 0;

}


void DX11VSMAtlas::Begin() {

	// Clear the atlas pages
	atlas_->ClearDepth(*immediate_context_);
	atlas_->ClearTargets(*immediate_context_, kOpaqueWhite);

	point_shadows_ = 0;

	immediate_context_->RSSetState(rasterizer_state_.Get());

}

bool DX11VSMAtlas::ComputeShadowmap(const PointLightComponent& point_light, const Scene& scene, PointShadow& shadow) {

	static const Vector2f uv_dimensions(0.50f, 0.25f);		// Simplification: each point light needs the same precision.

	if (!point_light.IsShadowEnabled()) {

		shadow.enabled = 0;
		return false;

	}

	auto light_transform = point_light.GetWorldTransform().inverse();

	shadow.atlas_page = 0;
	shadow.min_uv = Vector2f(uv_dimensions(0) * (point_shadows_ / 4),
							 uv_dimensions(1) * (point_shadows_ % 4));

	shadow.max_uv = shadow.min_uv + uv_dimensions;
	shadow.near_plane = 1.0f;
	shadow.far_plane = point_light.GetBoundingSphere().radius;
	shadow.light_view_matrix = light_transform.matrix();
	shadow.enabled = 1;

	// Render the geometry that can be seen from the light

	DrawShadowmap(shadow, 
				  scene.GetMeshHierarchy().GetIntersections(point_light.GetBoundingSphere()),
				  light_transform);

	++point_shadows_;

	return true;

}

void DX11VSMAtlas::DrawShadowmap(const PointShadow& shadow, const vector<VolumeComponent*>& nodes, const Affine3f& light_view_transform){

	D3D11_VIEWPORT view_port;

	view_port.TopLeftX = shadow.min_uv(0) * atlas_->GetWidth();
	view_port.TopLeftY = shadow.min_uv(1) * atlas_->GetHeight();
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.Width = (shadow.max_uv(0) - shadow.min_uv(0)) * atlas_->GetWidth();
	view_port.Height = (shadow.max_uv(1) - shadow.min_uv(1)) * atlas_->GetHeight();

	// Per-light setup

	auto& per_light_front = *per_light_->Lock<PerLight>();

	per_light_front.near_plane = shadow.near_plane;
	per_light_front.far_plane = shadow.far_plane;

	per_light_->Unlock();

	atlas_->Bind(*immediate_context_, shadow.atlas_page, &view_port);

	// Draw the geometry to the shadowmap

	DrawShadowmap(nodes, 
				  light_view_transform);

	// Cleanup

	atlas_->Unbind(*immediate_context_);

}

void DX11VSMAtlas::DrawShadowmap(const vector<VolumeComponent*> nodes, const Affine3f& light_view_transform) {
	
	ObjectPtr<DX11Mesh> mesh;

	for (auto&& node : nodes) {

		for (auto&& mesh_component : node->GetComponents<MeshComponent>()) {

			// Per-object setup

			mesh = mesh_component.GetMesh();

			mesh->Bind(*immediate_context_);

			immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);		// Override with 3-point patches

			auto& per_object = *per_object_->Lock<PerObject>();

			per_object.world_light = (light_view_transform * mesh_component.GetWorldTransform()).matrix();

			per_object_->Unlock();

			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index) {

				shadow_material_->Bind(*immediate_context_);

				// Draw	the subset

				DrawIndexedSubset(*immediate_context_,
								  mesh->GetSubset(subset_index));

			}

		}

	}

}

bool DX11VSMAtlas::ComputeShadowmap(const DirectionalLightComponent& directional_light, const Scene& scene, DirectionalShadow& shadow) {

	if (!directional_light.IsShadowEnabled()) {

		shadow.enabled = 0;
		return false;

	}

	shadow.enabled = false;

	return false;

}