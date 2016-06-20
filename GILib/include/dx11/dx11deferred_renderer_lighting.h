/// \file dx11deferred_renderer_lighting.h
/// \brief This file contains classes used to compute lighting for a deferred renderer under DirectX11
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>

#include "object.h"
#include "tag.h"
#include "dx11deferred_renderer_shared.h"

#include "dx11\dx11.h"
#include "windows\win_os.h"
#include "dx11voxelization.h"

namespace gi_lib {

	class IRenderTarget;
	class ITexture2D;
	class IGPTexture2DCache;
	class IRenderTargetCache;
	class IGPTexture2D;

	class PointLightComponent;
	class DirectionalLightComponent;
	class VolumeComponent;
	
	namespace dx11 {

		struct PointShadow;
		struct DirectionalShadow;

		class DX11Graphics;
		class DX11StructuredArray;
		class DX11StructuredBuffer;
		class DX11VSMAtlas;
		class DX11Computation;

		/// \brief Constant buffer used to pass parameters to the light accumulation shader.
		struct LightAccumulationParameters {

			Matrix4f inv_view_proj_matrix;			///< \brief Inverse view-projection matrix.

			Vector3f camera_position;				///< \brief Camera position in world space.

			float reserved;

			unsigned int point_lights;				///< \brief Amount of point lights.

			unsigned int directional_lights;		///< \brief Amount of directional lights.

		};

		/// \brief Describes a single point light.
		/// \remarks See "light_def.hlsl"
		struct PointLight {

			Vector4f position;			///< \brief Position of the light in world space.

			Vector4f color;				///< \brief Color of the light.

			float kc;					///< \brief Constant attenuation factor.

			float kl;					///< \brief Linear attenuation factor.

			float kq;					///< \brief Quadratic attenuation factor.

			float cutoff;				///< \brief Light minimum influence.

		};

		/// \brief Describes a single directional light.
		/// \remarks See "light_def.hlsl"
		struct DirectionalLight {

			Vector4f direction;			///< \brief Normal of the light in world space.

			Vector4f color;				///< \brief Color of the light.

		};

		/// \brief This class is used to calculate the lighting for a deferred renderer under DirectX11
		/// \author Raffaele D. Facendola
		class DX11DeferredRendererLighting {

		public:

			DX11DeferredRendererLighting(DX11Voxelization& voxelization);

			/// \brief No copy constructor.
			DX11DeferredRendererLighting(const DX11DeferredRendererLighting&) = delete;

			/// \brief Virtual destructor.
			virtual ~DX11DeferredRendererLighting();

			/// \brief No assignment operator.
			DX11DeferredRendererLighting& operator=(DX11DeferredRendererLighting&) = delete;

			/// \brief Accumulate the light from the specified light sources inside the light accumulation buffer.
			/// \param gbuffer GBuffer containing the scene
			/// \param lights Lights whose contribution needs to be accumulated.
			/// \param frame_info Information about the frame being rendered.
			ObjectPtr<ITexture2D> AccumulateLight(const ObjectPtr<IRenderTarget>& gbuffer, const std::vector<VolumeComponent*>& lights, const FrameInfo& frame_info);

		private:

			/// \brief Update the shadowmaps.
			/// \param lights Shadowcaster lights to update.
			/// \param frame_info Frame-specific info.
			/// \param point_lights_count Number of point lights among the provided light nodes.
			/// \param directional_lights_count Number of directional lights among the provided light nodes.
			void UpdateShadowmaps(const vector<VolumeComponent*>& lights, const FrameInfo &frame_info, unsigned int& point_lights_count, unsigned int& directional_lights_count);

			/// \brief Write the informations about a point light and its shadow.
			/// \param point_light Source light.
			/// \param light Contains the informations of the point light. Output.
			/// \param shadow Contains the informations of the point shadow. Output.
			void UpdateLight(const Scene& scene, const PointLightComponent& point_light, PointLight& light, PointShadow& shadow, bool light_injection);

			/// \brief Write the informations about a directional light and its shadow.
			/// \param directional_light Source light.
			/// \param aspect_ratio Aspect ratio of the client viewport.
			/// \param light Contains the informations of the directional light. Output.
			/// \param shadow Contains the informations of the directional shadow. Output.
			void UpdateLight(const Scene& scene, const DirectionalLightComponent& directional_light, float aspect_ratio, DirectionalLight& light, DirectionalShadow& shadow, bool light_injection);
			
			/// \brief Accumulate direct lighting.
			void AccumulateDirectLight(const ObjectPtr<IRenderTarget>& gbuffer, const FrameInfo &frame_info);

			/// \brief Accumulate indirect lighting.
			void AccumulateIndirectLight(const ObjectPtr<IRenderTarget>& gbuffer, const FrameInfo &frame_info);

			void FilterIndirectLight(const FrameInfo &frame_info);

			windows::COMPtr<ID3D11DeviceContext> immediate_context_;			///< \brief Immediate rendering context.

			DX11Graphics& graphics_;

			// Lights

			static const Tag kAlbedoEmissivityTag;								///< \brief Tag of the surface containing the albedo of the scene.

			static const Tag kNormalShininessTag;								///< \brief Tag of the surface containing the normal and the shininess of the scene.

			static const Tag kDepthStencilTag;									///< \brief Tag of the surface containing the depth stencil.

			static const Tag kPointLightsTag;									///< \brief Tag used to identify the array containing the point lights.

			static const Tag kDirectionalLightsTag;								///< \brief Tag used to identify the array containing the directional lights.

			static const Tag kLightBufferTag;									///< \brief Tag of the buffer used to accumulate light onto.

			static const Tag kLightParametersTag;								///< \brief Tag of the constant buffer used to pass light accumulation parameters.

            static const Tag kIndirectLightBufferTag;

			ObjectPtr<IGPTexture2DCache> gp_cache_;								///< \brief Cache of general purpose textures.

			ObjectPtr<IRenderTargetCache> rt_cache_;							///< \brief Cache of the render targets.

			ObjectPtr<IGPTexture2D> light_buffer_;								///< \brief Light buffer.

            ObjectPtr<IGPTexture2D> indirect_light_buffer_;						///< \brief Indirect light buffer.

			ObjectPtr<DX11StructuredArray> point_lights_;						///< \brief Array containing the point lights.

			ObjectPtr<DX11StructuredArray> directional_lights_;					///< \brief Array containing the directional lights.

			ObjectPtr<DX11StructuredBuffer> light_accumulation_parameters_;		///< \brief Constant buffer used to send light accumulation parameters to the shader.

			ObjectPtr<DX11Computation> light_shader_;							///< \brief Shader performing the light accumulation stage.

			// Shadows

			static const Tag kVSMShadowAtlasTag;								///< \brief Tag of the atlas containing the shadowmaps

			static const Tag kVSMSamplerTag;									///< \brief Tag of the sampler used to sample the VSM.

			static const Tag kPointShadowsTag;									///< \brief Tag used to identify the array containing the point shadows.

			static const Tag kDirectionalShadowsTag;							///< \brief Tag used to identify the array containing the directional shadows.

			std::unique_ptr<DX11VSMAtlas> shadow_atlas_;						///< \brief Contains the variance shadow maps.

			ObjectPtr<DX11StructuredArray> point_shadows_;						///< \brief Array containing the point lights.

			ObjectPtr<DX11StructuredArray> directional_shadows_;				///< \brief Array containing the directional lights.

			// Indirect lighting

			static const Tag kReflectiveShadowMapTag;							///< \brief Tag associated to the texture containing the albedo and the normal of the reflective shadow map.

			static const Tag kVarianceShadowMapTag;								///< \brief Tag associated to the texture containing the depth informations of the shadow map.

			DX11Voxelization& voxelization_;									///< \brief Used to perform scene voxelization, hold the lighting acceleration structure.

			ObjectPtr<DX11Computation> indirect_light_shader_;					///< \brief Shader performing the indirect light accumulation stage.

			ObjectPtr<DX11Computation> light_injection_;						///< \brief Shader performing the dynamic voxelization.

			ObjectPtr<DX11Computation> sh_filter_;								///< \brief Shader used to filter the SH data structure.

			ObjectPtr<DX11Computation> sh_convert_;								///< \brief Shader used to convert the monochromatic SH contribution to its final chromatic version.

			ObjectPtr<DX11StructuredBuffer> per_light_;							///< \brief Per-light constant buffer using during light injection.

			ObjectPtr<DX11StructuredBuffer> cb_point_light_;					///< \brief Constant buffer containing a single point light.

			ObjectPtr<DX11StructuredBuffer> cb_sh_filter;						///< \brief Constant buffer used to pass parameters to the SH MIP filter shader.

		};

	}

}