/// \file dx11deferred_renderer.h
/// \brief Deferred rendering classes for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include "deferred_renderer.h"

#include "instance_builder.h"
#include "dx11renderer.h"
#include "dx11graphics.h"
#include "dx11material.h"
#include "dx11buffer.h"
#include "dx11gpgpu.h"
#include "buffer.h"

#include "fx\dx11fx_lighting.h"


namespace gi_lib{

	class PointLightComponent;
	class DirectionalLightComponent;

	namespace dx11{

		struct PointShadow;
		struct DirectionalShadow;

		class DX11VSMAtlas;
		
		/// \brief Structure of the per-object constant buffer.
		struct VSPerObjectBuffer {

			Matrix4f world_view_proj;		///< \brief World * View * Projection matrix.

			Matrix4f world;					///< \brief World matrix.

		};

		/// \brief Constant buffer used to pass parameters to the light accumulation shader.
		struct LightAccumulationParameters{

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

		/// \brief Material for a DirectX11 deferred renderer.
		/// A custom material should not be compiled from code directly since there's no way of knowing whether the code is compatible with the custom renderer.
		/// A concrete deferred material is not a subclass of a DirectX11 material to prevent a DDoD. Composition is preferred here.
		/// \author Raffaele D. Facendola
		class DX11DeferredRendererMaterial : public DeferredRendererMaterial{

		public:

			/// \brief Create a new DirectX11 deferred material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11DeferredRendererMaterial(const CompileFromFile& args);

			virtual ObjectPtr<IMaterial> GetMaterial() override;

			virtual ObjectPtr<const IMaterial> GetMaterial() const override;

			ObjectPtr<DeferredRendererMaterial> Instantiate() const override;

			/// \brief Set the matrices needed to transform the object.
			void SetMatrix(const Affine3f& world, const Matrix4f& view_projection);

			/// \brief Bind the material to the pipeline.
			void Bind(ID3D11DeviceContext& context);

			virtual size_t GetSize() const override;

		private:

			DX11DeferredRendererMaterial(const ObjectPtr<DX11Material>& base_material);

			static const Tag kDiffuseMapTag;									///< \brief Tag associated to the diffuse map.

			static const Tag kDiffuseSampler;									///< \brief Tag associated to the sampler used to sample from the diffuse map.

			static const Tag kPerObjectTag;										///< \brief Tag associated to the per-object constant buffer.

			ObjectPtr<DX11StructuredBuffer> per_object_cbuffer_;				///< \brief Constant buffer containing the per-object constants used by the vertex shader.

			/// \brief Setup the material variables and resources.
			void Setup();

			ObjectPtr<DX11Material> material_;							///< \brief DirectX11 material.

		};

		/// \brief Deferred renderer with tiled lighting computation for DirectX11.
		/// \author Raffaele D. Facendola
		class DX11TiledDeferredRenderer : public TiledDeferredRenderer{

		public:

			/// \brief Create a new tiled deferred renderer.
			/// \param arguments Arguments used to construct the renderer.
			DX11TiledDeferredRenderer(const RendererConstructionArgs& arguments);

			/// \brief No copy constructor.
			DX11TiledDeferredRenderer(const DX11TiledDeferredRenderer&) = delete;

			/// \brief Virtual destructor.
			virtual ~DX11TiledDeferredRenderer();

			/// \brief No assignment operator.
			DX11TiledDeferredRenderer& operator=(DX11TiledDeferredRenderer&) = delete;

			virtual ObjectPtr<ITexture2D> Draw(unsigned int width, unsigned int height) override;

		private:

			/// \brief Information about the frame being rendered.
			struct FrameInfo {

				Scene* scene;

				CameraComponent* camera;

				Matrix4f view_proj_matrix;

				unsigned int width;

				unsigned int height;

				float aspect_ratio;
				
			};

			/// \brief Draw the current scene on the GBuffer.
			/// \param dimensions Dimensions of the GBuffer in pixels.
			void DrawGBuffer(const FrameInfo& frame_info);

			/// \brief Setup the GBuffer and bind it to the pipeline.
			/// \param dimensions Dimensions of the GBuffer in pixels.
			void BindGBuffer(const FrameInfo& frame_info);

			/// \brief Draws the specified nodes on the GBuffer.
			/// \param nodes Nodes to draw.
			/// \param frame_info Information about the frame being rendered.
			void DrawNodes(const vector<VolumeComponent*>& meshes, const FrameInfo& frame_info);
			
			/// \param dimensions Dimensions of the LightBuffer in pixels.
			/// \param frame_info Information about the frame being rendered.
			void ComputeLighting(const FrameInfo& frame_info);

			/// \brief Accumulate the light from the specified light sources inside the light accumulation buffer.
			/// \param lights Lights whose contribution needs to be accumulated.
			/// \param frame_info Information about the frame being rendered.
			void AccumulateLight(const vector<VolumeComponent*>& lights, const FrameInfo& frame_info);

			/// \brief Write the informations about a point light and its shadow.
			/// \param point_light Source light.
			/// \param light Contains the informations of the point light. Output.
			/// \param shadow Contains the informations of the point shadow. Output.
			void UpdateLight(const PointLightComponent& point_light, PointLight& light, PointShadow& shadow);

			/// \brief Write the informations about a directional light and its shadow.
			/// \param directional_light Source light.
			/// \param light Contains the informations of the directional light. Output.
			/// \param shadow Contains the informations of the directional shadow. Output.
			void UpdateLight(const DirectionalLightComponent& directional_light, DirectionalLight& light, DirectionalShadow& shadow);
			
			/// \param dimensions Dimensions of the Exposed buffer in pixels.
			void ComputePostProcess(const FrameInfo& frame_info);

			// Render context

			COMPtr<ID3D11DeviceContext> immediate_context_;						///< \brief Immediate rendering context.

			COMPtr<ID3D11DepthStencilState> depth_state_;						///< \brief Depth-stencil buffer state.

			COMPtr<ID3D11BlendState> blend_state_;								///< \brief Output merger blending state.

			COMPtr<ID3D11RasterizerState> rasterizer_state_;					///< \brief Rasterizer state.

			COMPtr<ID3D11DepthStencilState> disable_depth_test_;

			// GBuffer
			
			ObjectPtr<DX11RenderTarget> gbuffer_;								///< \brief GBuffer.

			// Light accumulation

			static const Tag kAlbedoTag;										///< \brief Tag of the surface containing the albedo of the scene.

			static const Tag kNormalShininessTag;								///< \brief Tag of the surface containing the normal and the shininess of the scene.

			static const Tag kDepthStencilTag;									///< \brief Tag of the surface containing the depth stencil.

			static const Tag kPointLightsTag;									///< \brief Tag used to identify the array containing the point lights.

			static const Tag kDirectionalLightsTag;								///< \brief Tag used to identify the array containing the directional lights.

			static const Tag kLightBufferTag;									///< \brief Tag of the buffer used to accumulate light onto.

			static const Tag kLightParametersTag;								///< \brief Tag of the constant buffer used to pass light accumulation parameters.

			ObjectPtr<IGPTexture2D> light_buffer_;								///< \brief Light buffer.

			ObjectPtr<DX11StructuredArray> point_lights_;						///< \brief Array containing the point lights.

			ObjectPtr<DX11StructuredArray> directional_lights_;					///< \brief Array containing the directional lights.

			ObjectPtr<DX11StructuredBuffer> light_accumulation_parameters_;		///< \brief Constant buffer used to send light accumulation parameters to the shader.

			ObjectPtr<DX11Computation> light_shader_;							///< \brief Shader performing the light accumulation stage.

			// Shadows

			static const Tag kVSMShadowAtlasTag;								///< \brief Tag of the atlas containing the shadowmaps

			static const Tag kVSMSamplerTag;									///< \brief Tag of the sampler used to sample the VSM.

			static const Tag kPointShadowsTag;									///< \brief Tag used to identify the array containing the point shadows.

			static const Tag kDirectionalShadowsTag;							///< \brief Tag used to identify the array containing the directional shadows.

			unique_ptr<DX11VSMAtlas> shadow_atlas_;								///< \brief Contains the variance shadow maps.

			ObjectPtr<DX11StructuredArray> point_shadows_;						///< \brief Array containing the point lights.

			ObjectPtr<DX11StructuredArray> directional_shadows_;				///< \brief Array containing the directional lights.

			// Post process

			fx::DX11FxBloom fx_bloom_;											///< \brief Performs the bloom filter of the image.

			ObjectPtr<IRenderTarget> bloom_output_;								///< \brief Contains the result of the bloom filter.

			fx::DX11FxTonemap fx_tonemap_;										///< \brief Performs the tonemapping of the image.

			ObjectPtr<IGPTexture2D> tonemap_output_;							///< \brief Contains the result of the tonemapping.
			
		};

		/////////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////////

		INSTANTIABLE(DeferredRendererMaterial, DX11DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile);

		inline ObjectPtr<IMaterial> DX11DeferredRendererMaterial::GetMaterial(){

			return ObjectPtr<IMaterial>(material_);

		}

		inline ObjectPtr<const IMaterial> gi_lib::dx11::DX11DeferredRendererMaterial::GetMaterial() const{

			return ObjectPtr<const IMaterial>(material_);

		}

		inline void gi_lib::dx11::DX11DeferredRendererMaterial::Bind(ID3D11DeviceContext& context){

			material_->Bind(context);

		}

		inline size_t gi_lib::dx11::DX11DeferredRendererMaterial::GetSize() const{

			return material_->GetSize();

		}

		///////////////////////////////////// DX11 TILED DEFERRED RENDERER //////////////////////////////////

		INSTANTIABLE(TiledDeferredRenderer, DX11TiledDeferredRenderer, RendererConstructionArgs);

	}

}
