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

	/// \brief Component used to draw object using a deferred renderer.
	/// The component will store one material per mesh subset.
	class DeferredRendererComponent : public Component{

	public:

		/// \brief No default constructor.
		DeferredRendererComponent() = delete;

		/// \brief Create a new deferred renderer component.
		/// \param mesh_component Mesh component to draw.		
		DeferredRendererComponent(MeshComponent& mesh_component);

		/// \brief No assignment operator.
		DeferredRendererComponent& operator=(const DeferredRendererComponent&) = delete;

		/// \brief Virtual destructor.
		virtual ~DeferredRendererComponent();

		/// \brief Get the mesh component.
		/// \return Returns the mesh component.
		ObjectPtr<IStaticMesh> GetMesh();

		/// \brief Get the mesh component.
		/// \return Returns the mesh component.
		ObjectPtr<const IStaticMesh> GetMesh() const;

		/// \brief Get the material count.
		/// \return Returns the material count.
		unsigned int GetMaterialCount() const;

		/// \brief Get a material.
		/// \param material_index Index of the material to retrieve.
		/// \return Returns the specified material.
		ObjectPtr<DeferredRendererMaterial> GetMaterial(unsigned int material_index);

		/// \brief Get a material.
		/// \param material_index Index of the material to retrieve.
		/// \return Returns the specified material.
		ObjectPtr<const DeferredRendererMaterial> GetMaterial(unsigned int material_index) const;

		/// \brief Get the world transform.
		/// \return Return the world transform associated to the transform component.
		const Affine3f& GetWorldTransform() const;

		/// \brief Set a new material.
		/// \param material_index Index of the material to set.
		/// \param material New material
		void SetMaterial(unsigned int material_index, ObjectPtr<DeferredRendererMaterial> material);

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		MeshComponent& mesh_component_;									///< \brief Mesh component to draw (must belong to the same object)

		TransformComponent* transform_component_;						///< \brief Transform component used to draw the mesh (must belong to the same object)

		unique_ptr<Listener> on_mesh_removed_listener_;					///< \brief Detects whether the mesh component have been removed from the object.

		vector<ObjectPtr<DeferredRendererMaterial>> materials_;			///< \brief List of materials (one per mesh subset)

	};

	/// \brief Deferred renderer with tiled lighting computation.
	/// \author Raffaele D. Facendola
	class TiledDeferredRenderer : public IRenderer{

	public:
				
		/// \brief Create a new tiled deferred renderer.
		/// \param scene Scene assigned to the renderer.
		TiledDeferredRenderer(Scene& scene);

		/// \brief No assignment operator.
		TiledDeferredRenderer& operator=(const TiledDeferredRenderer&) = delete;

		/// \brief Virtual destructor.
		virtual ~TiledDeferredRenderer();

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

	////////////// DEFERRED RENDERER COMPONENT /////////////////

	inline const Affine3f& DeferredRendererComponent::GetWorldTransform() const {

		return transform_component_->GetWorldTransform();

	}

}



