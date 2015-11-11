/// \file deferred_renderer.h
/// \brief Deferred rendering classes.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>

#include "graphics.h"
#include "material.h"
#include "component.h"
#include "scene.h"
#include "observable.h"
#include "mesh.h"

using ::std::vector;
using ::std::shared_ptr;
using ::std::unique_ptr;

namespace gi_lib{

	/// \brief Exposes additional informations for a material used by a deferred renderer.
	/// \author Raffaele D. Facendola
	class DeferredRendererMaterial : public IResource{

	public:

		/// \brief Structure used to compile a deferred material from a file.
		/// This structure is identical to the one used by the base material.
		using CompileFromFile = IMaterial::CompileFromFile;

		/// \brief Structure used to instantiate an existing material.
		struct Instantiate{

			NO_CACHE;

			ObjectPtr<DeferredRendererMaterial> base;	///< \brief Material to instantiate.

		};
		
		/// \brief Virtual destructor.
		virtual ~DeferredRendererMaterial();

		/// \brief Get the base material.
		/// \return Returns the base material.
		virtual ObjectPtr<IMaterial> GetMaterial() = 0;

		/// \brief Get the base material.
		/// \return Returns the base material.
		virtual ObjectPtr<const IMaterial> GetMaterial() const = 0;

		/// \brief Instantiate this material.
		/// \return Returns a new instance of this material.
		virtual ObjectPtr<DeferredRendererMaterial> Instantiate() const = 0;

	};

	/// \brief Renderer with deferred lighting computation.
	/// \author Raffaele D. Facendola
	class DeferredRenderer : public IRenderer{

	public:
				
		/// \brief Create a new tiled deferred renderer.
		/// \param scene Scene assigned to the renderer.
		DeferredRenderer(Scene& scene);

		/// \brief No assignment operator.
		DeferredRenderer& operator=(const DeferredRenderer&) = delete;

		/// \brief Virtual destructor.
		virtual ~DeferredRenderer();

		/// \brief Get the scene this renderer is assigned to.
		/// \return Returns the scene this renderer is assigned to.
		virtual Scene& GetScene() override;

		/// \brief Get the scene this renderer is assigned to.
		/// \return Returns the scene this renderer is assigned to.
		virtual const Scene& GetScene() const override;

	private:

		Scene& scene_;		///< \brief Scene this render refers to.

	};

	////////////// DEFERRED RENDERER MATERIAL ////////////////

	inline DeferredRendererMaterial::~DeferredRendererMaterial(){}

}



