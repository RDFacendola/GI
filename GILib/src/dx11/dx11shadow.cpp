#include "dx11/dx11shadow.h"

#include <algorithm>

#include "dx11/dx11graphics.h"
#include "dx11/dx11mesh.h"

#include "core.h"
#include "light_component.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;

#undef max
#undef min

namespace {
	
	/// \brief Vertex shader constant buffer used to draw the geometry from the light perspective.
	struct VSMPerObjectCBuffer {

		Matrix4f world_light;								///< \brief World * Light-view matrix for point lights,
															///			World * Light-view * Light-proj matrix for directional lights

	};

	/// \brief Pixel shader constant buffer used to project the fragments to shadow space.
	struct VSMPerLightCBuffer {

		float near_plane;									///< \brief Near clipping plane.

		float far_plane;									///< \brief Far clipping plane.

	};
		
	/// \brief Find the smallest chunk which can accommodate the given size in the given chunk array.
	/// If the method succeeds the best chunk is then moved at the end of the collection.
	/// \param size Size of the square region to accommodate.
	/// \return Returns returns the iterator to the best chunk in the given list. If non such chunk can be found the method returns an iterator to the end of the given collection
	template <typename TIterator>
	TIterator GetBestChunk(const Vector2i& size, TIterator begin, TIterator end) {

		auto best_chunk = end;

		for (auto it = begin; it != end; ++it) {

			if(it->sizes()(0) + 1 >= size(0) &&									// Fits horizontally
			   it->sizes()(1) + 1 >= size(1) &&									// Fits vertically
			   (best_chunk == end ||
			    it->sizes().maxCoeff() < best_chunk->sizes().maxCoeff())){		// Just an heuristic guess of "smallest"

				best_chunk = it;

			}

		}

		if (best_chunk != end) {

			// Swap the last element of the range with the best chunk and returns

			std::swap(*best_chunk,
					  *(--end));

		}

		
		return end;
		
	}
	
	/// \brief Splits the given chunk along each direction according to the requested size and returns the remaining chunks.
	/// The reserved chunk starts always from the top-left corner of the chunk.
	/// \param size Size of the reserved chunk.
	/// \param chunk Chunk to split.
	vector<AlignedBox2i> SplitChunk(const Vector2i& size, const AlignedBox2i& chunk) {

		// The reserved chunk is the top-left.
		vector<AlignedBox2i> bits;

		if (chunk.sizes()(0) + 1 > size(0)) {

			bits.push_back(AlignedBox2i(Vector2i(chunk.min()(0) + size(0), chunk.min()(1)),
										Vector2i(chunk.max()(0), chunk.min()(1) + size(1) - 1)));

		}

		if (chunk.sizes()(1) + 1 > size(1)) {

			bits.push_back(AlignedBox2i(Vector2i(chunk.min()(0), chunk.min()(1) + size(1)),
										chunk.max()));

		}

		return bits;

	}

	/// \brief Reserve a free chunk in the given chunk list and returns informations about the reserved chunk
	/// \param size Size of the region to reserve.
	/// \param chunks List of free chunks for each atlas page.
	/// \param page_index If the method succeeds, this parameter contains the atlas page where the chunk has been reserved.
	/// \param reserved_chunk if the method succeeds, this parameter contains the boundaries of the reserved chunk.
	/// \return Returns true if the method succeeds, return false otherwise.
	bool ReserveChunk(const Vector2i& size, vector<vector<AlignedBox2i>>& chunks, unsigned int& page_index, AlignedBox2i& reserved_chunk) {

		page_index = 0;

		// A directional shadow requires only one chunk in any atlas page

		for (auto&& page_chunks : chunks) {

			auto it = GetBestChunk(size, 
								   page_chunks.begin(), 
								   page_chunks.end());

			if (it != page_chunks.end()) {

				reserved_chunk = AlignedBox2i(it->min(),
											  it->min() + size - Vector2i::Ones());

				// Update the chunk list

				auto bits = SplitChunk(size, *it);

				page_chunks.pop_back();

				page_chunks.insert(page_chunks.end(),
								   bits.begin(),
								   bits.end());
				
				return true;

			}

			++page_index;

		}

		return false;

	}

	/// \brief Reserve a free chunk in the given chunk list and fill the proper shadow information.
	/// \param size Size of the region to reserve.
	/// \param chunks List of free chunks for each atlas page.
	/// \param shadow If the method succeeds, this object contains the updated shadow informations.
	/// \return Returns true if the method succeeds, return false otherwise.
	bool ReserveChunk(const Vector2i& size, const Vector2i& atlas_size, vector<vector<AlignedBox2i>>& chunks, PointShadow& shadow) {

		unsigned int page_index;
		AlignedBox2i reserved_chunk;

		if (ReserveChunk(size.cwiseProduct(Vector2i(2,1)),		// A dual-paraboloid shadowmap requires twice the size to store both the front and the rear shadowmaps
						 chunks, 
						 page_index, 
						 reserved_chunk)) {

			auto uv_size = (atlas_size - Vector2i::Ones()).cast<float>();

			shadow.atlas_page = page_index;
			
			shadow.min_uv = reserved_chunk.min().cast<float>().cwiseQuotient(uv_size);
			shadow.max_uv = reserved_chunk.max().cast<float>().cwiseQuotient(uv_size);

			return true;

		}

		return false;

	}

	/// \brief Reserve a free chunk in the given chunk list and fill the proper shadow information.
	/// \param size Size of the region to reserve.
	/// \param chunks List of free chunks for each atlas page.
	/// \param shadow If the method succeeds, this object contains the updated shadow informations.
	/// \return Returns true if the method succeeds, return false otherwise.
	bool ReserveChunk(const Vector2i& size, const Vector2i& atlas_size, vector<vector<AlignedBox2i>>& chunks, DirectionalShadow& shadow) {

		unsigned int page_index;
		AlignedBox2i reserved_chunk;

		if (ReserveChunk(size, 
						 chunks, 
						 page_index, 
						 reserved_chunk)) {

			auto uv_size = (atlas_size - Vector2i::Ones()).cast<float>();

			shadow.atlas_page = page_index;
			
			shadow.min_uv = reserved_chunk.min().cast<float>().cwiseQuotient(uv_size);
			shadow.max_uv = reserved_chunk.max().cast<float>().cwiseQuotient(uv_size);

			return true;

		}

		return false;

	}
	
	/// \brief Get the minimum and the maximum depth along the specified direction for any given mesh in the provided volume collection.
	/// \param volumes Volumes collection to cycle through
	/// \param direction Depth direction.
	/// \return Returns a 2-element vector where the first element is the minimum depth found and the second is the maximum one.
	Vector2f GetZRange(const vector<VolumeComponent*>& volumes, const Vector3f& direction) {

		Vector2f range(std::numeric_limits<float>::infinity(),			// Min
					   -std::numeric_limits<float>::infinity());		// Max

		float distance;

		for (auto&& volume : volumes) {

			for (auto&& mesh_component : volume->GetComponents<MeshComponent>()) {

				auto& sphere = mesh_component.GetBoundingSphere();

				distance = sphere.center.dot(direction);

				if (distance - sphere.radius < range(0)) {

					range(0) = distance - sphere.radius;

				}

				if (distance + sphere.radius > range(1)) {

					range(1) = distance + sphere.radius;

				}

			}

		}
		
		return range;

	}

	/// \brief Get the light frustum associated to the specified directional light.
	/// The frustum is computed based on the assumption that the affected geometry is the one inside the camera's frustum only.
	/// \param directional_light Light whose frustum will be computed.
	/// \param camera The view camera.
	/// \param ortho_size The computed orthographic size. Output, optional.
	/// \return Returns the frustum associated to the directional light.
	Frustum GetLightFrustum(const DirectionalLightComponent& directional_light, const CameraComponent& camera, float aspect_ratio, Vector2f* ortho_size) {

		if (camera.GetProjectionType() == ProjectionType::Perspective) {

			// Take the diameter of the view frustum as upper bound of the orthographic size - This assumes that the light's position is in the center of the frustum

			float far_height = 2.0f * camera.GetMaximumDistance() * std::tanf(camera.GetFieldOfView() * 0.5f);		// Height of the far plane

			float diameter = Vector3f(far_height * aspect_ratio, far_height, camera.GetMaximumDistance()).norm() / 3.0f;	

			float half_diameter = diameter * 0.5f;

			if (ortho_size) {

				*ortho_size = Vector2f(diameter, diameter);		// Equal for each direction

			}

			// Create the frustum
			auto& camera_transform = camera.GetTransformComponent();

			auto& light_transform = *directional_light.GetComponent<TransformComponent>();
			
			Vector3f frustum_center = camera_transform.GetPosition() + camera_transform.GetForward() * (camera.GetMinimumDistance() + camera.GetMaximumDistance()) * 0.5f;

			Vector3f light_forward = light_transform.GetForward();
			Vector3f light_right = light_transform.GetRight();
			Vector3f light_up = light_transform.GetUp();

			Vector3f domain_size = 15000.f * Vector3f::Identity();

			// Create the frustum
			return Frustum({ Math::MakePlane( light_forward, frustum_center + light_forward.cwiseProduct(domain_size)),			// Near clipping plane. The projection range is infinite.
							 Math::MakePlane(-light_forward, frustum_center - light_forward.cwiseProduct(domain_size)),			// Far clipping plane. The projection range is infinite.
							 Math::MakePlane(-light_right, frustum_center + light_right * half_diameter),						// Right clipping plane
							 Math::MakePlane( light_right, frustum_center - light_right * half_diameter),						// Left clipping plane
							 Math::MakePlane(-light_up, frustum_center + light_up * half_diameter),								// Top clipping plane
							 Math::MakePlane( light_up, frustum_center - light_up * half_diameter) });							// Bottom clipping plane
			
		}
		else {

			THROW(L"Not supported, buddy!");

		}
		
	}

}

///////////////////////////////////// DX11 VSM ATLAS /////////////////////////////////////////

const Tag DX11VSMAtlas::kPerObject = "PerObject";
const Tag DX11VSMAtlas::kPerLight = "PerLight";

DX11VSMAtlas::DX11VSMAtlas(unsigned int size, unsigned int pages, bool full_precision) :
	fx_blur_(1.67f){

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

	sampler_ = new DX11Sampler(ISampler::FromDescription{ TextureMapping::CLAMP, TextureFiltering::ANISOTROPIC, 4 });

	auto format = full_precision ? TextureFormat::RG_FLOAT : TextureFormat::RG_HALF;

	atlas_ = new DX11RenderTargetArray(IRenderTargetArray::FromDescription{ size,
																		    size, 
																		    pages,
																		    format } );
	
	blur_atlas_ = new DX11GPTexture2DArray(ITexture2DArray::FromDescription{ size,
																			 size, 
																			 pages,
																			 1, 
																			 format } );

	dpvsm_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\paraboloid_vsm.hlsl" });

	vsm_material_ = new DX11Material(IMaterial::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\vsm.hlsl" });

	per_object_ = new DX11StructuredBuffer(sizeof(VSMPerObjectCBuffer));

	per_light_ = new DX11StructuredBuffer(sizeof(VSMPerLightCBuffer));

	// One-time setup

	dpvsm_material_->SetInput(kPerObject,
							  ObjectPtr<IStructuredBuffer>(per_object_));

	dpvsm_material_->SetInput(kPerLight,
							  ObjectPtr<IStructuredBuffer>(per_light_));

	vsm_material_->SetInput(kPerObject,
							ObjectPtr<IStructuredBuffer>(per_object_));
	
}

void DX11VSMAtlas::Begin() {

	// Clear the atlas pages - TODO: Clear by need

	atlas_->ClearDepth(*immediate_context_);
	atlas_->ClearTargets(*immediate_context_, kOpaqueWhite);
	
	immediate_context_->RSSetState(rasterizer_state_.Get());

	// Clear any existing chunk and starts over (basically we restore one big chunk for each atlas page)

	chunks_.resize(atlas_->GetCount());

	for (auto&& page_chunk : chunks_) {

		page_chunk.clear();
		page_chunk.push_back(AlignedBox2i(Vector2i::Zero(),
										  Vector2i(atlas_->GetWidth() - 1, 
												   atlas_->GetHeight() - 1)));

	}

}

void DX11VSMAtlas::Commit() {

	// Unbind the previously bound atlas

	atlas_->Unbind(*immediate_context_);

	// Blur the shadowmaps contained inside the atlas and store the result inside the blurred version of the atlas.

	fx_blur_.Blur(atlas_->GetRenderTargets(),
				  ObjectPtr<IGPTexture2DArray>(blur_atlas_));
	
}

bool DX11VSMAtlas::ComputeShadowmap(const PointLightComponent& point_light, const Scene& scene, PointShadow& shadow, float near_plane, float far_plane) {

	if (!point_light.IsShadowEnabled() ||
		!ReserveChunk(point_light.GetShadowMapSize(),
					  Vector2i(atlas_->GetWidth(), atlas_->GetHeight()),
					  chunks_,
					  shadow)){

		shadow.enabled = 0;
		return false;

	}
	
	auto light_transform = point_light.GetWorldTransform().inverse();

	// Fill the remaining shadow infos

	shadow.near_plane = std::max(0.0f, near_plane);
	shadow.far_plane = std::min(point_light.GetBoundingSphere().radius, far_plane);
	shadow.light_view_matrix = light_transform.matrix();

	shadow.enabled = 1;

	// Draw the actual shadowmap

	DrawShadowmap(shadow, 
				  scene.GetMeshHierarchy().GetIntersections(point_light.GetBoundingSphere()),
				  light_transform);
		
	return true;

}

bool DX11VSMAtlas::ComputeShadowmap(const DirectionalLightComponent& directional_light, const Scene& scene, DirectionalShadow& shadow, float aspect_ratio, float far_plane) {
		
	if (!directional_light.IsShadowEnabled() ||
		!ReserveChunk(directional_light.GetShadowMapSize(),
					  Vector2i(atlas_->GetWidth(), atlas_->GetHeight()),
					  chunks_,
					  shadow)){
		
		shadow.enabled = 0;
		return false;

	}

	// Calculate the light's boundaries

	Vector2f ortho_size(10000.f, 10000.f);

	auto& camera = *scene.GetMainCamera();

	Sphere domain{ Vector3f::Zero(), 15000.f };

	auto lit_geometry = scene.GetMeshHierarchy().GetIntersections(domain);

	
	auto z_range = GetZRange(lit_geometry,
							 directional_light.GetDirection());

	z_range(1) = std::min(z_range(1), z_range(0) + far_plane);		// Clamp the maximum depth wrt the minimum depth found.

	auto light_world_transform = directional_light.GetWorldTransform().matrix();

/*
	light_world_transform.col(3) = Math::ToVector4(camera.GetTransformComponent().GetPosition() + camera.GetTransformComponent().GetForward() * (camera.GetMaximumDistance() * 0.5f),
												   1.0f);*/

	auto light_transform = ComputeOrthographicProjectionLH(ortho_size(0),
														   ortho_size(1),
														   z_range(0),
										 				   z_range(1)) * light_world_transform.inverse();
	
	// Fill the remaining shadow infos

	shadow.light_view_matrix = light_transform;

	shadow.enabled = 1;

	// Draw the actual shadowmap

	DrawShadowmap(shadow,
				  lit_geometry,
				  light_transform);

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

	auto& per_light_front = *per_light_->Lock<VSMPerLightCBuffer>();

	per_light_front.near_plane = shadow.near_plane;
	per_light_front.far_plane = shadow.far_plane;

	per_light_->Unlock();

	atlas_->Bind(*immediate_context_, 
				 shadow.atlas_page, 
				 &view_port);

	// Draw the geometry to the shadowmap

	DrawShadowmap(nodes, 
				  dpvsm_material_,
				  light_view_transform.matrix(),
				  true);
	
}

void DX11VSMAtlas::DrawShadowmap(const DirectionalShadow& shadow, const vector<VolumeComponent*>& nodes, const Matrix4f& light_proj_transform) {
	
	D3D11_VIEWPORT view_port;

	view_port.TopLeftX = shadow.min_uv(0) * atlas_->GetWidth();
	view_port.TopLeftY = shadow.min_uv(1) * atlas_->GetHeight();
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.Width = (shadow.max_uv(0) - shadow.min_uv(0)) * atlas_->GetWidth();
	view_port.Height = (shadow.max_uv(1) - shadow.min_uv(1)) * atlas_->GetHeight();

	atlas_->Bind(*immediate_context_,
				 shadow.atlas_page,
				 &view_port);

	// Draw the geometry to the shadowmap

	DrawShadowmap(nodes,
				  vsm_material_,
				  light_proj_transform);

}

void DX11VSMAtlas::DrawShadowmap(const vector<VolumeComponent*> nodes, const ObjectPtr<DX11Material>& shadow_material, const Matrix4f& light_transform, bool tessellable) {
	
	ObjectPtr<DX11Mesh> mesh;
	
	shadow_material->Bind(*immediate_context_);

	for (auto&& node : nodes) {

		for (auto&& mesh_component : node->GetComponents<MeshComponent>()) {

			// Per-object setup

			mesh = mesh_component.GetMesh();

			mesh->Bind(*immediate_context_,
					   tessellable);

			auto& per_object = *per_object_->Lock<VSMPerObjectCBuffer>();

			per_object.world_light = (light_transform * mesh_component.GetWorldTransform()).matrix();

			per_object_->Unlock();

			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index) {

				shadow_material->Commit(*immediate_context_);

				// Draw	the subset

				mesh->DrawSubset(*immediate_context_, 
								 subset_index);

			}

		}

	}

	shadow_material->Unbind(*immediate_context_);

}

