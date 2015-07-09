#include "..\include\material_importer.h"

#include "texture.h"
#include "object.h"

using namespace gi;
using namespace gi_lib;
using namespace gi_lib::fbx;

namespace{

	/// \brief Bind a fbx property to a shader texture 2d.
	/// \param resources Object used to load the proper 
	bool BindTexture2D(Resources& resources, unique_ptr<IFbxProperty> fbx_property, const Tag& texture_semantic, IMaterial& material, const wstring& base_directory){

		if (fbx_property){

			ObjectPtr<ITexture2D> texture;

			for (auto&& texture_name : fbx_property->EnumerateTextures()){

				texture = resources.Load<ITexture2D, ITexture2D::FromFile>({ base_directory + to_wstring(texture_name) });

				if (texture){

					material.SetInput(texture_semantic,
									  texture);

					return true;

				}

			}

		}

		return false;

	}

	/// \brief Instantiate a concrete material.
	ObjectPtr<DeferredRendererMaterial> InstantiateMaterial(Resources& resources, ObjectPtr<DeferredRendererMaterial> base_material, IFbxMaterial& fbx_material, const wstring& base_directory) {

		ObjectPtr<DeferredRendererMaterial> deferred_material_instance = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::Instantiate>({ base_material });

		auto material_instance = deferred_material_instance->GetMaterial();

		// Diffuse map

		//Use this when importing model from 3ds max: fbx_material["3dsMax|Parameters|diff_color_map"]
		BindTexture2D(resources,
					  fbx_material["DiffuseColor"],
					  "gDiffuseMap",
					  *material_instance,
					  base_directory);

		// Okay

		return deferred_material_instance;

	}

}
MaterialImporter::MaterialImporter(Resources& resources) :
resources_(resources){

	auto& app = Application::GetInstance();

	base_material_ = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ app.GetDirectory() + L"Data\\gbuffer.hlsl" });

}

void MaterialImporter::OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh){

	// Add a renderer component for the deferred renderer.
	auto deferred_component = mesh.AddComponent<DeferredRendererComponent>(mesh);

	// Instantiate the proper materials for each mesh subset.

	for (unsigned int material_index = 0; material_index < deferred_component->GetMaterialCount(); ++material_index){

		deferred_component->SetMaterial(material_index,
										InstantiateMaterial(resources_,
															base_material_,
															*materials[material_index],
															base_directory));

	}

}