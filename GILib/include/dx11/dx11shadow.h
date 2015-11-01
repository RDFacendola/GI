/// \file dx11shadow.h
/// \brief This file defines classes used to handle and manage shadows.
///
/// \author Raffaele D. Facendola

#pragma once

#include "object.h"
#include "dx11sampler.h"
#include "dx11render_target.h"
#include "dx11material.h"

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

			int atlas_page;							///< \brief Index of the page inside the atlas containing the shadowmap.

			int enabled;							///< \brief Whether the shadow is enabled (!0) or not (0).

		};

		/// \brief Describes the shadow casted by a single directional light.
		/// \remarks See "shadow_def.hlsl"
		struct DirectionalShadow {

			Matrix4f light_view_matrix;				///< \brief Matrix used to transform from world-space to light-view-space.

			Vector2f min_uv;						///< \brief Minimum uv coordinates inside the shadowmap page.

			Vector2f max_uv;						///< \brief Maximum uv coordinates inside the shadowmap page.

			int atlas_page;							///< \brief Index of the page inside the atlas containing the shadowmap.

			int enabled;							///< \brief Whether the shadow is enabled (!0) or not (0).

			Vector2f reserved;

		};

		/// \brief Class used to store together multiple variance shadow maps.
		/// \author Raffaele. D Facendola
		class DX11VSMAtlas {

		public:

			/// \brief Create a new VSM shadow atlas.
			DX11VSMAtlas(unsigned int width, unsigned height, unsigned int pages, bool full_precision = false);

			/// \brief Clear the atlas from any existing shadowmap.
			void Restore();

			/// \brief Computes a variance shadowmap.
			/// \param point_light Point light casting the shadow.
			/// \param scene Scene containing the caster geometry.
			/// \param shadow Structure containing the data used to access the shadowmap for the HLSL code.
			/// \return Returns true if the shadowmap was calculated correctly, returns false otherwise.
			bool ComputeShadowmap(const PointLightComponent& point_light, const Scene& scene, PointShadow& shadow);

			/// \brief Computes a variance shadowmap.
			/// \param directional_light Point light casting the shadow.
			/// \param scene Scene containing the caster geometry.
			/// \param shadow Structure containing the data used to access the shadowmap for the HLSL code.
			/// \return Returns true if the shadowmap was calculated correctly, returns false otherwise.
			bool ComputeShadowmap(const DirectionalLightComponent& directional_light, const Scene& scene, DirectionalShadow& shadow);

			/// \brief Get the shadow atlas.
			ObjectPtr<ITexture2DArray> GetAtlas();

			/// \brief Get the default sampler used to sample the atlas.
			ObjectPtr<DX11Sampler> GetSampler();

		private:
						
			static const Tag kPerObject;							///< \brief Tag of the per-object constant buffer.

			static const Tag kPerLight;								///< \brief Tag of the per-light constant buffer.

			struct PerObject {

				Matrix4f world_light;								///< \brief World * Light-view matrix.

			};

			struct PerLight {

				float near_plane;									///< \brief Near clipping plane.

				float far_plane;									///< \brief Far clipping plane.

				float front_factor;

			};

			void DrawShadowmap(const vector<VolumeComponent*> nodes, const Affine3f& light_view_transform);

			COMPtr<ID3D11DeviceContext> immediate_context_;			///< \brief Immediate rendering context.

			ObjectPtr<DX11RenderTargetArray> atlas_;				///< \brief Array of render targets used as shadow atlas.

			ObjectPtr<DX11Sampler> sampler_;						///<\ brief Sampler used to sample the VSM.

			ObjectPtr<DX11Material> shadow_material_;				///< \brief Material used to write the VSM.

			ObjectPtr<DX11StructuredBuffer> per_object_;			///< \brief Per-object constant buffer.

			ObjectPtr<DX11StructuredBuffer> per_light_;				///< \brief Per-light constant buffer.

		};

		/////////////////////////////////////// DX11 VSM SHADOW ATLAS ///////////////////////////////////////

		inline ObjectPtr<ITexture2DArray> DX11VSMAtlas::GetAtlas() {

			return atlas_->GetRenderTargets();

		}
				
		inline ObjectPtr<DX11Sampler> DX11VSMAtlas::GetSampler() {

			return sampler_;

		}

	}

}
