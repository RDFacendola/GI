/// \file material_importer.h
/// \brief Class to handle the material import.
///
/// \author Raffaele D. Facendola

#pragma once

#include <gilib.h>
#include <core.h>
#include <scene.h>
#include <resources.h>

#include "fbx/fbx.h"
#include "wavefront/wavefront_obj.h"

#include "deferred_renderer.h"
#include "sampler.h"


using gi_lib::ObjectPtr;

using gi_lib::Application;
using gi_lib::Resources;
using gi_lib::MeshComponent;

using gi_lib::DeferredRendererMaterial;

using gi_lib::wavefront::IMtlMaterialImporter;
using gi_lib::wavefront::IMtlMaterial;

using gi_lib::fbx::IFbxMaterialImporter;
using gi_lib::fbx::IFbxMaterial;
using gi_lib::fbx::FbxMaterialCollection;

using gi_lib::IMaterial;
using gi_lib::Tag;

namespace gi{

	/// \brief Handle the material import from a obj file and a mtl material library file.
	/// \author Raffaele. D. Facendola
	class MtlMaterialImporter : public IMtlMaterialImporter {

	public:

		/// \brief Create a new material importer.
		/// \param resources Factory used to load and instantiate materials.
		MtlMaterialImporter(Resources& resources);

		virtual void OnImportMaterial(const wstring& base_directory, const IMtlMaterial& material, MeshComponent& mesh) override;

	private:
		
		bool BindTexture(const wstring& base_directory, const IMtlMaterial& mtl_material, const string& mtl_property, const Tag& semantic, IMaterial& destination) const;

		bool BindProperty(const IMtlMaterial& mtl_material, const string& mtl_property, float default_value, float& destination);

		Resources& resources_;											///< \brief Used to load various materials.

		ObjectPtr<DeferredRendererMaterial> base_material_;				///< \brief Base material for every game object.

		ObjectPtr<gi_lib::ISampler> sampler_;							///< \brief Basic sampler used by the material.

	};

	/// \brief Handle the material import from a fbx file.
	/// \author Raffaele. D. Facendola
	class FbxMaterialImporter : public IFbxMaterialImporter{

	public:

		/// \brief Create a new material importer.
		/// \param resources Factory used to load and instantiate materials.
		FbxMaterialImporter(Resources& resources);

		virtual void OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh) override;

	private:
		
		Resources& resources_;											///< \brief Used to load various materials.

		ObjectPtr<DeferredRendererMaterial> base_material_;				///< \brief Base material for every game object.

		ObjectPtr<gi_lib::ISampler> sampler_;							///< \brief Basic sampler used by the material.

	};

}