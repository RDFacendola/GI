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

			virtual ObjectPtr<MaterialVariable> GetVariable(const string& name) override;

			virtual ObjectPtr<MaterialResource> GetResource(const string& name) override;

			virtual size_t GetSize() const override;

			/// \brief Get the base material.
			/// \return Returns a pointer to the base material.
			ObjectPtr<DX11Material> GetMaterial();

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

		};

		/////////////////////////////// DX11 DEFERRED RENDERER MATERIAL /////////////////////////

		inline ObjectPtr<Material::MaterialVariable> DX11DeferredRendererMaterial::GetVariable(const string& name){

			return material_->GetVariable(name);

		}

		inline ObjectPtr<Material::MaterialResource> DX11DeferredRendererMaterial::GetResource(const string& name){

			return material_->GetResource(name);

		}
		
		inline size_t DX11DeferredRendererMaterial::GetSize() const{

			return material_->GetSize();

		}

		inline ObjectPtr<DX11Material> DX11DeferredRendererMaterial::GetMaterial(){

			return material_;

		}

	}

}
