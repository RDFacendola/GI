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

	// Create the shadow resources

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, 16 });

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

}

void DX11VSMAtlas::Restore() {

	// Clear the atlas pages
	atlas_->ClearDepth(*immediate_context_);
	atlas_->ClearTargets(*immediate_context_, kOpaqueWhite);			// Technically not needed

}

bool DX11VSMAtlas::ComputeShadowmap(const PointLightComponent& point_light, const Scene& scene, PointShadow& shadow) {

	if (!point_light.IsShadowEnabled()) {

		shadow.enabled = 0;
		return false;

	}

	auto light_transform = point_light.GetWorldTransform().inverse();

	// Simplification hypothesis: point lights have 2 1024x1024 shadowmaps.
	
	shadow.atlas_page = 0;
	shadow.min_uv = Vector2f::Zero();
	shadow.max_uv = Vector2f::Ones().cwiseProduct(Vector2f(1.00f, 0.50f));
	shadow.near_plane = 0.1f;
	shadow.far_plane = 4000.0f;
	shadow.light_view_matrix = light_transform.matrix();
	shadow.enabled = 1;

	// Render the geometry that can be seen from the light

	auto nodes = scene.GetMeshHierarchy().GetIntersections(point_light.GetBoundingSphere());
	
	D3D11_VIEWPORT front_viewport;
	D3D11_VIEWPORT rear_viewport;

	Vector2f top_left = shadow.min_uv.cwiseProduct(Vector2f(atlas_->GetWidth(), atlas_->GetHeight()));

	Vector2f dimensions = (shadow.max_uv - shadow.min_uv).cwiseProduct(Vector2f(atlas_->GetWidth() * 0.5f, atlas_->GetHeight()));

	// FRONT

	front_viewport.TopLeftX = top_left(0);
	front_viewport.TopLeftY = top_left(1);
	front_viewport.MinDepth = 0.0f;
	front_viewport.MaxDepth = 1.0f;
	front_viewport.Width = dimensions(0);
	front_viewport.Height = dimensions(1);

	auto& per_light_front = *per_light_->Lock<PerLight>();

	per_light_front.near_plane = shadow.near_plane;
	per_light_front.far_plane = shadow.far_plane;
	per_light_front.front_factor = 1.0f;

	per_light_->Unlock();

	atlas_->Bind(*immediate_context_, shadow.atlas_page, &front_viewport);

	DrawShadowmap(nodes, light_transform);

	// REAR

	rear_viewport.TopLeftX = top_left(0) + dimensions(0);
	rear_viewport.TopLeftY = top_left(1);
	rear_viewport.MinDepth = 0.0f;
	rear_viewport.MaxDepth = 1.0f;
	rear_viewport.Width = dimensions(0);
	rear_viewport.Height = dimensions(1);

	auto& per_light_rear = *per_light_->Lock<PerLight>();

	per_light_rear.near_plane = shadow.near_plane;
	per_light_rear.far_plane = shadow.far_plane;
	per_light_rear.front_factor = -1.0f;

	per_light_->Unlock();

	atlas_->Bind(*immediate_context_, shadow.atlas_page, &rear_viewport);

	DrawShadowmap(nodes, light_transform);

	// CLEAR

	atlas_->Unbind(*immediate_context_);
	
	return true;

}

void DX11VSMAtlas::DrawShadowmap(const vector<VolumeComponent*> nodes, const Affine3f& light_view_transform) {
	
	ObjectPtr<DX11Mesh> mesh;

	for (auto&& node : nodes) {

		for (auto&& mesh_component : node->GetComponents<MeshComponent>()) {

			// Draw each subset with the shadow material

			mesh = mesh_component.GetMesh();

			mesh->Bind(*immediate_context_);

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