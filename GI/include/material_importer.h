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

#include "deferred_renderer.h"

using gi_lib::ObjectPtr;

using gi_lib::Application;
using gi_lib::Resources;
using gi_lib::MeshComponent;

using gi_lib::DeferredRendererMaterial;

using gi_lib::fbx::IFbxMaterialImporter;
using gi_lib::fbx::IFbxMaterial;
using gi_lib::fbx::FbxMaterialCollection;

namespace gi{


	/// \brief Handle the material import from a fbx file.
	/// \author Raffaele. D. Facendola
	class MaterialImporter : public IFbxMaterialImporter{

	public:

		/// \brief Create a new material importer.
		/// \param resources Factory used to load and instantiate materials.
		MaterialImporter(Resources& resources);

		virtual void OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh) override;

	private:
		
		Resources& resources_;											///< \brief Used to load various materials.

		ObjectPtr<DeferredRendererMaterial> base_material_;				///< \brief Base material for every game object.

	};

}