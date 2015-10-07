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


namespace gi_lib{

	namespace dx11{

		/// \brief Structure of the per-object constant buffer.
		struct VSPerObjectBuffer {

			Matrix4f world_view_proj;		// World * View * Projection matrix.

			Matrix4f world;					// World matrix.

		};

		/// \brief Parameters used by the tonemapper
		struct TonemapParams{

			float vignette;					// Vignette factor.

			float exposure_mul;				// Multiplicative exposure factor.

			float exposure_add;				// Additive exposure factor.

		};

		struct CSPointLight {

			Vector4f position;

			Vector4f color;

			float linear_decay;

			float square_decay;

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

			/// \brief Instantiate a DirectX11 deferred material from another one.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to instantiate the material.
			DX11DeferredRendererMaterial(const Instantiate& args);

			virtual ObjectPtr<IMaterial> GetMaterial() override;

			virtual ObjectPtr<const IMaterial> GetMaterial() const override;

			/// \brief Set the matrices needed to transform the object.
			void SetMatrix(const Affine3f& world, const Matrix4f& view_projection);

			/// \brief Bind the material to the pipeline.
			void Bind(ID3D11DeviceContext& context);

			virtual size_t GetSize() const override;

		private:

			static const Tag kDiffuseMapTag;									///< \brief Tag associated to the diffuse map.

			static const Tag kDiffuseSampler;									///< \brief Tag associated to the sampler used to sample from the diffuse map.

			static const Tag kPerObjectTag;										///< \brief Tag associated to the per-object constant buffer.

			ObjectPtr<StructuredBuffer<VSPerObjectBuffer>> per_object_cbuffer_;	///< \brief Constant buffer containing the per-object constants used by the vertex shader.

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

			/// \brief Draw the current scene on the GBuffer.
			/// \param dimensions Dimensions of the GBuffer in pixels.
			void DrawGBuffer(unsigned int width, unsigned int height);

			/// \brief Setup the GBuffer and bind it to the pipeline.
			/// \param dimensions Dimensions of the GBuffer in pixels.
			void BindGBuffer(unsigned int width, unsigned int height);

			/// \brief Draws the specified nodes on the GBuffer.
			/// \param nodes Nodes to draw.
			/// \param view_projection_matrix View projection matrix used to transform the nodes in projection space.
			void DrawNodes(const vector<VolumeComponent*>& nodes, const Matrix4f& view_projection_matrix);

			void SetupLights();

			/// \param dimensions Dimensions of the LightBuffer in pixels.
			void ComputeLighting(unsigned int width, unsigned int height);

			/// \param dimensions Dimensions of the Exposed buffer in pixels.
			void ComputeTonemap(unsigned int width, unsigned int height);

			// Render context

			COMPtr<ID3D11DeviceContext> immediate_context_;				///< \brief Immediate rendering context.

			COMPtr<ID3D11DepthStencilState> depth_state_;				///< \brief Depth-stencil buffer state.

			COMPtr<ID3D11BlendState> blend_state_;						///< \brief Output merger blending state.

			COMPtr<ID3D11RasterizerState> rasterizer_state_;			///< \brief Rasterizer state.

			COMPtr<ID3D11DepthStencilState> disable_depth_test_;

			// GBuffer
			
			ObjectPtr<DX11RenderTarget> gbuffer_;						///< \brief GBuffer.

			// Light accumulation

			static const Tag kAlbedoTag;								///< \brief Tag of the surface containing the albedo of the scene.

			static const Tag kNormalShininessTag;						///< \brief Tag of the surface containing the normal and the shininess of the scene.

			static const Tag kLightBufferTag;							///< \brief Tag of the buffer used to accumulate light onto.

			static const Tag kPointLightsTag;							///< \brief Tag used to identify the array containing point lights.

			ObjectPtr<IGPTexture2D> light_buffer_;						///< \brief Light buffer.

			ObjectPtr<DX11StructuredArray> point_lights_;				///< \brief Array containing the lights.

			ObjectPtr<DX11Computation> light_shader_;					///< \brief Shader performing the light accumulation stage.

			// Tonemapping

			static const Tag kTonemapParamsTag;							///< \brief Tag of the constant buffer containing the tonemapping parameters

			static const Tag kUnexposedParamsTag;						///< \brief Tag of the unexposed buffer used as input of the tonemapping.

			static const Tag kExposedParamsTag;							///< \brief Tag of the exposed buffer used as output of the tonemapping.

			ObjectPtr<DX11StructuredBuffer> tonemap_params_;			///< \brief Buffer containing the tonemap parameters.

			ObjectPtr<DX11Computation> tonemap_shader_;					///< \brief Shader performing the tonemapping stage.

			ObjectPtr<IGPTexture2D> exposed_buffer_;					///< \brief Contains the result of the tonemapping.

		};

		/////////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////////

		INSTANTIABLE(DeferredRendererMaterial, DX11DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile);
		INSTANTIABLE(DeferredRendererMaterial, DX11DeferredRendererMaterial, DeferredRendererMaterial::Instantiate);

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
