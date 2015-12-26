#include "..\include\material_importer.h"

#include "texture.h"
#include "object.h"

using namespace gi;
using namespace gi_lib;
using namespace gi_lib::fbx;

namespace{

	struct PerMaterial {

		float gShininess;			// Material shininess
		float gEmissivity;			// Material emissivity (self-illumination)

		Vector2f reserved;

	};

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
		static const Tag kNormalMapTag = "gNormalMap";
		
		auto material_instance = base_material->Instantiate();

		// Diffuse map

		//gisponza: Use this when importing model from 3ds max: fbx_material["3dsMax|Parameters|diff_color_map"]
		//oldsponza: fbx_material["DiffuseColor"]

		BindTexture2D(resources,
					  fbx_material["DiffuseColor"],
					  kDiffuseMapTag,
					  *material_instance->GetMaterial(),
					  base_directory);
		
		return material_instance;

	}



}

/////////////////////////////////////// MTL MATERIAL IMPORTER /////////////////////////////////////////

MtlMaterialImporter::MtlMaterialImporter(Resources& resources) :
resources_(resources) {

	using gi_lib::ISampler;

	auto& app = Application::GetInstance();

	base_material_ = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\gbuffer.hlsl" });

	sampler_ = resources.Load<ISampler, ISampler::FromDescription>({ TextureMapping::WRAP, TextureFiltering::ANISOTROPIC, 16 });

}

void MtlMaterialImporter::OnImportMaterial(const wstring& base_directory, const MtlMaterialCollection& material_collection, MeshComponent& mesh){
	
	// Add a renderer component for the deferred renderer.
	auto deferred_component = mesh.AddComponent<AspectComponent<DeferredRendererMaterial>>(mesh);

	// Instantiate the proper materials for each mesh subset.

	for (unsigned int material_index = 0; material_index < deferred_component->GetMaterialCount(); ++material_index) {

		auto material_instance = base_material_->Instantiate();

		auto per_material = resources_.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(PerMaterial) });
	
		auto& buffer = *per_material->Lock<PerMaterial>();
		
		auto& material = *material_collection[material_index];

		BindTexture(base_directory, material, "map_Kd", "gDiffuseMap", *material_instance->GetMaterial());
		BindTexture(base_directory, material, "map_bump", "gNormalMap", *material_instance->GetMaterial());
		BindTexture(base_directory, material, "map_Ks", "gSpecularMap", *material_instance->GetMaterial());

		BindProperty(material, "Ns", 5.0f, buffer.gShininess);
		BindProperty(material, "Ke", 0.0f, buffer.gEmissivity);
	
		per_material->Unlock();

		if(!material_instance->GetMaterial()->SetInput("PerMaterial", 
													   ObjectPtr<IStructuredBuffer>(per_material))){
		
			THROW(L"Unable to find PerMaterial constant buffer!");

		}

		if (!material_instance->GetMaterial()->SetInput("gDiffuseSampler",
														sampler_)) {

			THROW(L"Unable to find gDiffuseSampler sampler state!");

		}
		
		deferred_component->SetMaterial(material_index,
										material_instance);

	}
	

}

bool MtlMaterialImporter::BindProperty(const IMtlMaterial& mtl_material, const string& mtl_property, float default_value, float& destination) {

	float property_value;

	auto property = mtl_material[mtl_property];

	if (property &&
		property->Read(property_value)) {

		destination = property_value;

		return true;

	}

	destination = default_value;

	return true;

}

bool MtlMaterialImporter::BindTexture(const wstring& base_directory, const IMtlMaterial& mtl_material, const string& mtl_property, const Tag& semantic, IMaterial& destination) const{

	string texture_name;

	auto property_map_kd = mtl_material[mtl_property];

	if (property_map_kd &&
		property_map_kd->Read(texture_name)) {

		auto texture = resources_.Load<ITexture2D, ITexture2D::FromFile>({ base_directory + to_wstring(texture_name) });

		if (texture) {

			return destination.SetInput(semantic,
										texture);

		}

	}

	return false;

}

/////////////////////////////////////// FBX MATERIAL IMPORTER /////////////////////////////////////////

FbxMaterialImporter::FbxMaterialImporter(Resources& resources) :
resources_(resources){

	using gi_lib::ISampler;

	auto& app = Application::GetInstance();

	base_material_ = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ app.GetDirectory() + L"Data\\Shaders\\gbuffer.hlsl" });

	sampler_ = resources.Load<ISampler, ISampler::FromDescription>({TextureMapping::WRAP, TextureFiltering::ANISOTROPIC, 16});

}

void FbxMaterialImporter::OnImportMaterial(const wstring& base_directory, FbxMaterialCollection& materials, MeshComponent& mesh){

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
