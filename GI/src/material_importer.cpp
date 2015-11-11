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

					return material.SetInput(texture_semantic,
											 texture);

				}

			}

		}

		return false;

	}

	/// \brief Instantiate a concrete material.
	ObjectPtr<DeferredRendererMaterial> InstantiateMaterial(Resources& resources, ObjectPtr<DeferredRendererMaterial> base_material, IFbxMaterial& fbx_material, const wstring& base_directory) {

		static const Tag kDiffuseMapTag = "gDiffuseMap";
		
		auto material_instance = base_material->Instantiate();

		// Diffuse map

		//Use this when importing model from 3ds max: fbx_material["3dsMax|Parameters|diff_color_map"]
		BindTexture2D(resources,
					  fbx_material["DiffuseColor"],
					  kDiffuseMapTag,
					  *material_instance->GetMaterial(),
					  base_directory);
		
		return material_instance;

	}

}
MaterialImporter::MaterialImporter(Resources& resources) :
resources_(resources){

	using gi_lib::ISampler;

	auto& app = Application::GetInstance();

	base_material_ = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\gbuffer.hlsl" });

	sampler_ = resources.Load<ISampler, ISampler::FromDescription>({TextureMapping::WRAP, 16});

}

void MaterialImporter::OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh){

	static const Tag kSamplerTag = "gDiffuseSampler";

	// Add a renderer component for the deferred renderer.
	auto deferred_component = mesh.AddComponent<AspectComponent<DeferredRendererMaterial>>(mesh);

	// Instantiate the proper materials for each mesh subset.

	for (unsigned int material_index = 0; material_index < deferred_component->GetMaterialCount(); ++material_index){

		auto material = InstantiateMaterial(resources_,
											base_material_,
											*materials[material_index],
											base_directory);

		material->GetMaterial()->SetInput(kSamplerTag, sampler_);

		deferred_component->SetMaterial(material_index,
										material);

	}

}