/// \file dx11deferred_renderer.h
/// \brief Deferred rendering classes for DirectX11.
///
/// \author Raffaele D. Facendola

#pragma once

#include "dx11renderer.h"

#include "..\dx11graphics.h"

#include "..\..\..\include\renderers\deferred_renderer.h"

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

			/// \brief Instantiate a DirectX11 deferred material from another one.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to instantiate the material.
			DX11DeferredRendererMaterial(const Instantiate& args);

			virtual ObjectPtr<Material> GetMaterial() override;

			virtual ObjectPtr<const Material> GetMaterial() const override;

			virtual size_t GetSize() const override;

		private:

			ObjectPtr<DX11Material> material_;		///< \brief DirectX11 material.

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

			virtual void Draw(IOutput& output) override;

		private:

			unique_ptr<ID3D11DeviceContext, COMDeleter> immediate_context_;		///< \brief Immediate rendering context.

			unique_ptr<ID3D11DepthStencilState, COMDeleter> depth_state_;		///< \brief Depth-stencil buffer state.

			unique_ptr<ID3D11BlendState, COMDeleter> blend_state_;				///< \brief Output merger blending state.

			unique_ptr<ID3D11RasterizerState, COMDeleter> rasterizer_state_;	///< \brief Rasterizer state.
		};

		/////////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////////

		inline ObjectPtr<Material> DX11DeferredRendererMaterial::GetMaterial()
		{

			return material_;

		}

		inline size_t gi_lib::dx11::DX11DeferredRendererMaterial::GetSize() const
		{

			return material_->GetSize();

		}

		inline ObjectPtr<const Material> gi_lib::dx11::DX11DeferredRendererMaterial::GetMaterial() const
		{

			return material_;

		}

	}

}
