/// \file dx11shadow.h
/// \brief This file defines classes used to handle and manage shadows.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"

#include "dx11sampler.h"
#include "dx11render_target.h"
#include "dx11material.h"

#include "fx\dx11fx_filter.h"

namespace gi_lib {

	class PointLightComponent;
	class DirectionalLightComponent;
	class VolumeComponent;

	namespace dx11 {
				
		/// \brief Describes the shadow casted by a single point light.
		/// \remarks See "shadow_def.hlsl"
		struct PointShadow {

			Matrix4f light_view_matrix;				///< \brief Matrix used to transform from world-space to light-view-space.

			Vector2f min_uv;						///< \brief Minimum uv coordinates inside the shadowmap page.

			Vector2f max_uv;						///< \brief Maximum uv coordinates inside the shadowmap page.

			float near_plane;						///< \brief Near clipping plane of the light.

			float far_plane;						///< \brief Far clipping plane of the light.

			unsigned int atlas_page;				///< \brief Index of the page inside the atlas containing the shadowmap.

			int enabled;							///< \brief Whether the shadow is enabled (!0) or not (0).

		};

		/// \brief Describes the shadow casted by a single directional light.
		/// \remarks See "shadow_def.hlsl"
		struct DirectionalShadow {

			Matrix4f light_view_matrix;				///< \brief Matrix used to transform from world-space to light-view-space.

			Vector2f min_uv;						///< \brief Minimum uv coordinates inside the shadowmap page.

			Vector2f max_uv;						///< \brief Maximum uv coordinates inside the shadowmap page.

			unsigned int atlas_page;				///< \brief Index of the page inside the atlas containing the shadowmap.

			int enabled;							///< \brief Whether the shadow is enabled (!0) or not (0).

			Vector2f reserved;						///< \brief For padding purposes only.

		};

		/// \brief Class used to store together multiple variance shadow maps.
		/// \author Raffaele. D Facendola
		class DX11VSMAtlas {

		public:

			/// \brief Create a new VSM shadow atlas.
			DX11VSMAtlas(unsigned int size/*, unsigned int pages*/, bool full_precision = false);

			/// \brief Reset the current status of the shadowmap atlas.
			void Reset();

			/// \brief Computes a variance shadowmap.
			/// \param point_light Point light casting the shadow.
			/// \param scene Scene containing the caster geometry.
			/// \param shadow Structure containing the data used to access the shadowmap for the HLSL code.
			/// \param shadow_map If the method succeeds, it contains the computed VSM prior to the soft shadows stage. Optional.
			/// \return Returns true if the shadowmap was calculated correctly, returns false otherwise.
			bool ComputeShadowmap(const PointLightComponent& point_light, const Scene& scene, PointShadow& shadow, ObjectPtr<IRenderTarget>* shadow_map = nullptr);
			
			/// \brief Computes a variance shadowmap.
			/// \param directional_light Point light casting the shadow.
			/// \param scene Scene containing the caster geometry.
			/// \param shadow Structure containing the data used to access the shadowmap for the HLSL code.
			/// \param shadow_map If the method succeeds, it contains the computed VSM prior to the soft shadows stage. Optional.
			/// \return Returns true if the shadowmap was calculated correctly, returns false otherwise.
			bool ComputeShadowmap(const DirectionalLightComponent& directional_light, const Scene& scene, DirectionalShadow& shadow, ObjectPtr<IRenderTarget>* shadow_map = nullptr);
			
			/// \brief Get the shadow atlas.
			ObjectPtr<ITexture2D> GetAtlas();

			/// \brief Get the default sampler used to sample the atlas.
			ObjectPtr<DX11Sampler> GetSampler();

		private:
						
			void DrawShadowmap(const PointShadow& shadow, const vector<VolumeComponent*>& nodes, const Matrix4f& light_view_transform, ObjectPtr<IRenderTarget>* shadow_map);

			void DrawShadowmap(const DirectionalShadow& shadow, const vector<VolumeComponent*>& nodes, const Matrix4f& light_proj_transform, ObjectPtr<IRenderTarget>* shadow_map);

			void DrawShadowmap(const AlignedBox2i& boundaries, unsigned int atlas_page, const vector<VolumeComponent*> nodes, const ObjectPtr<DX11Material>& shadow_material, const Matrix4f& light_transform, ObjectPtr<IRenderTarget>* shadow_map, bool tessellable = false);

			COMPtr<ID3D11DeviceContext> immediate_context_;			///< \brief Immediate rendering context.

			COMPtr<ID3D11RasterizerState> rs_depth_bias_;			///< \brief Depth biased rasterizer state.

			vector<vector<AlignedBox2i>> chunks_;					///< \brief Contains the free chunks for each atlas page.
																	///			A chunk is a free region of space within the atlas.

			ObjectPtr<DX11GPTexture2D> atlas_;						///< \brief Contains the packed shadowmaps to be used on the next frame.

			ObjectPtr<DX11Sampler> sampler_;						///<\ brief Sampler used to sample the VSM.

			ObjectPtr<DX11Material> point_shadow_material_;			///< \brief Material used for point light shadows.

			ObjectPtr<DX11Material> directional_shadow_material_;	///< \brief Material used for directional light shadows.

			ObjectPtr<DX11StructuredBuffer> per_object_;			///< \brief Per-object constant buffer.

			ObjectPtr<DX11StructuredBuffer> per_light_;				///< \brief Per-light constant buffer.
			
			std::unique_ptr<DX11RenderTargetCache> rt_cache_;		///< \brief Pointer to the render-target texture cache.

			DX11FxGaussianBlur fx_blur_;							///< \brief Used to blur the shadowmap.
			
		};

		/////////////////////////////////////// DX11 VSM SHADOW ATLAS ///////////////////////////////////////

		inline ObjectPtr<ITexture2D> DX11VSMAtlas::GetAtlas() {

			return atlas_->GetTexture();

		}
				
		inline ObjectPtr<DX11Sampler> DX11VSMAtlas::GetSampler() {

			return sampler_;

		}

	}

}
