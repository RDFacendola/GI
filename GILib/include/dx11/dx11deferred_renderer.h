/// \file dx11deferred_renderer.h
/// \brief Deferred rendering classes for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include "deferred_renderer.h"

#include <memory>

#include "instance_builder.h"
#include "dx11renderer.h"
#include "dx11graphics.h"
#include "dx11material.h"
#include "dx11buffer.h"
#include "dx11gpgpu.h"
#include "buffer.h"

#include "dx11deferred_renderer_shared.h"
#include "dx11deferred_renderer_lighting.h"
#include "dx11voxelization.h"

namespace gi_lib{

	namespace dx11{

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

			virtual size_t GetSize() const override;

			/// \brief Set the matrices needed to transform the object.
			void SetMatrix(const Affine3f& world, const Matrix4f& view_projection);

			/// \brief Bind the material to the pipeline.
			void Bind(ID3D11DeviceContext& context);
			
		private:
			
			/// \brief Constant buffer used to pass the parameters to the shader.
			struct ShaderParameters {

				Matrix4f world_view_proj;		///< \brief World * View * Projection matrix.

				Matrix4f world;					///< \brief World matrix.

			};

			DX11DeferredRendererMaterial(const ObjectPtr<DX11Material>& base_material);

			static const Tag kShaderParameters;									///< \brief Tag associated to the per-object constant buffer.

			ObjectPtr<DX11Material> material_;									///< \brief Underlying DirectX11 material.

			ObjectPtr<DX11StructuredBuffer> shader_parameters_;					///< \brief Constant buffer containing the per-object constants used by the material shader.
			
		};

		/// \brief Deferred renderer with tiled lighting computation for DirectX11.
		/// \author Raffaele D. Facendola
		class DX11DeferredRenderer : public DeferredRenderer{

		public:

			/// \brief Create a new tiled deferred renderer.
			/// \param arguments Arguments used to construct the renderer.
			DX11DeferredRenderer(const RendererConstructionArgs& arguments);

			/// \brief No copy constructor.
			DX11DeferredRenderer(const DX11DeferredRenderer&) = delete;

			/// \brief Virtual destructor.
			virtual ~DX11DeferredRenderer();

			/// \brief No assignment operator.
			DX11DeferredRenderer& operator=(DX11DeferredRenderer&) = delete;

			virtual ObjectPtr<ITexture2D> Draw(const Time& time, unsigned int width, unsigned int height) override;

			virtual void EnableGlobalIllumination(bool enable /* = true */) override;
			
			Matrix4f GetViewProjectionMatrix(float aspect_ratio) const;

			virtual ObjectPtr<ITexture2D> DrawVoxels(const ObjectPtr<ITexture2D>& image) override;

			virtual ObjectPtr<ITexture2D> DrawSH(const ObjectPtr<ITexture2D>& image) override;

			virtual void LockCamera(bool lock) override;

		private:

			/// \brief Draw the current scene on the GBuffer.
			/// \param dimensions Dimensions of the GBuffer in pixels.
			void DrawGBuffer(const FrameInfo& frame_info);

			/// \brief Draws the specified nodes on the GBuffer.
			/// \param nodes Nodes to draw.
			/// \param frame_info Information about the frame being rendered.
			void DrawNodes(const vector<VolumeComponent*>& meshes, const FrameInfo& frame_info);
			
			/// \param dimensions Dimensions of the LightBuffer in pixels.
			/// \param frame_info Information about the frame being rendered.
			ObjectPtr<ITexture2D> ComputeLighting(const FrameInfo& frame_info);

			DX11Graphics& graphics_;

			// Render context

			COMPtr<ID3D11DeviceContext> immediate_context_;						///< \brief Immediate rendering context.

			// GBuffer
			
			ObjectPtr<IRenderTargetCache> rt_cache_;							///< \brief Cache of render targets.

			ObjectPtr<DX11RenderTarget> gbuffer_;								///< \brief GBuffer.

			// Light accumulation

			std::unique_ptr<DX11DeferredRendererLighting> lighting_;			///< \brief Object used to calculate scene lighting.

			// Global illumination

			bool enable_global_illumination_;									///< \brief Whether to enable the global illumination.

			std::unique_ptr<DX11Voxelization> voxelization_;					///< \brief Used to calculate the dynamic voxelization of the scene.

			// Debug

			bool lock_camera_;													///< \brief Whether the camera is locked or not.

			CameraComponent* locked_camera_;									///< \brief The locked camera.

		};

		///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

		INSTANTIABLE(DeferredRendererMaterial, DX11DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile);

		inline ObjectPtr<IMaterial> DX11DeferredRendererMaterial::GetMaterial(){

			return ObjectPtr<IMaterial>(material_);

		}

		inline ObjectPtr<const IMaterial> DX11DeferredRendererMaterial::GetMaterial() const{

			return ObjectPtr<const IMaterial>(material_);

		}

		inline void DX11DeferredRendererMaterial::Bind(ID3D11DeviceContext& context){

			material_->Bind(context);

		}

		inline size_t DX11DeferredRendererMaterial::GetSize() const{

			return material_->GetSize();

		}
		
		///////////////////////////////// DX11 DEFERRED RENDERER //////////////////////////////////

		INSTANTIABLE(DeferredRenderer, DX11DeferredRenderer, RendererConstructionArgs);

		inline void DX11DeferredRenderer::EnableGlobalIllumination(bool enable) {

			enable_global_illumination_ = enable;

		}

		inline ObjectPtr<ITexture2D> DX11DeferredRenderer::DrawVoxels(const ObjectPtr<ITexture2D>& image) {

			return voxelization_->DrawVoxels(image);

		}

		inline ObjectPtr<ITexture2D> DX11DeferredRenderer::DrawSH(const ObjectPtr<ITexture2D>& image) {

			return voxelization_->DrawSH(image);

		}

		inline void DX11DeferredRenderer::LockCamera(bool lock) {

			lock_camera_ = lock;

		}

	}

}
