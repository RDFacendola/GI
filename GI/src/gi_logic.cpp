#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>
#include <renderers\deferred_renderer.h>
#include <spatial hierarchy\uniform_tree.h>

#include <component.h>
#include <range.h>

#include <scene.h>
#include <fbx\fbx.h>
#include <Eigen/Geometry>

#include <typeindex>
#include <typeinfo>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::fbx;
using namespace ::Eigen;

/// \brief Title of the main window.
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

/// \brief Size of the domain (for each edge).
const float kDomainSize = 1000.0f;

/// \brief Number of times the domain is split along each axis.
const unsigned int kDomainSubdivisions = 3;

class MaterialImporter : public IMaterialImporter{

public:

	/// \brief Create a new material importer.
	/// \param resources Factory used to load and instantiate materials.
	MaterialImporter(Resources& resources) :
	resources_(resources){

		base_material_ = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ Application::GetDirectory() + L"Data\\deferred_material.fx" });

		base_texture_ = resources.Load<Texture2D, Texture2D::FromFile>({ Application::GetDirectory() + L"Data\\textures\\gi_flag.DDS" });

	}

	virtual void OnImportMaterial(MaterialCollection& materials, MeshComponent& mesh){

		// Add a renderer component for the deferred renderer.
		auto deferred_component = mesh.AddComponent<DeferredRendererComponent>(mesh);

		// Instantiate the proper materials for each mesh subset.

		for (unsigned int material_index = 0; material_index < deferred_component->GetMaterialCount(); ++material_index){

			deferred_component->SetMaterial(material_index,
											InstantiateMaterial(*materials[material_index]));
			
		}

	}
		
private:

	/// \brief Instantiate a concrete material.
	ObjectPtr<DeferredRendererMaterial> InstantiateMaterial(IMaterial& material){

		ObjectPtr<DeferredRendererMaterial> material_instance = resources_.Load<DeferredRendererMaterial, DeferredRendererMaterial::Instantiate>({ base_material_ });

		auto base_material = material_instance->GetMaterial();

		auto ps_map = base_material->GetResource("ps_map");

		ps_map->Set(base_texture_->GetView());

		// Set the proper textures...

		return material_instance;

	}

	Resources& resources_;											///< \brief Used to load various materials.

	ObjectPtr<DeferredRendererMaterial> base_material_;				///< \brief Base material for every game object.

	ObjectPtr<Texture2D> base_texture_;								///< \brief Dummy texture.

};

GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11)),
scene_(make_unique<UniformTree>(AABB{Vector3f::Zero(),
									 kDomainSize * Vector3f::Ones() },
								kDomainSubdivisions * Vector3i::Ones()))
{

	// Graphics setup

	SetTitle(kWindowTitle);

	Show();
	
	// Create the output window

	output_ = graphics_.CreateOutput(*this, 
									 graphics_.GetAdapterProfile().video_modes[0]);

	// Create the renderers

	deferred_renderer_ = std::move(graphics_.CreateRenderer<TiledDeferredRenderer>(scene_));

	// Camera setup

	auto camera = scene_.CreateNode(L"MainCamera",
									Translation3f(Vector3f::Ones()),
									Quaternionf::Identity(),
									AlignedScaling3f(Vector3f::Ones()))
						->AddComponent<CameraComponent>();

	camera->SetProjectionType(ProjectionType::Perspective);
	camera->SetMinimumDistance(1.0f);
	camera->SetMaximumDistance(1000.0f);
	camera->SetFieldOfView(Math::DegToRad(45.0f));

	scene_.SetMainCamera(camera);

	// Scene import

	auto node = scene_.CreateNode(L"root", 
								  Translation3f(Vector3f::Zero()), 
								  Quaternionf::Identity(), 
								  AlignedScaling3f(Vector3f::Ones()));

	auto& resources = graphics_.GetResources();

	MaterialImporter material_importer(resources);

	FbxImporter fbx_importer(material_importer,
							 resources);
	
	fbx_importer.ImportScene(Application::GetDirectory() + L"Data\\gisponza.fbx", 
							 *node);
	
}

GILogic::~GILogic(){

	output_ = nullptr;

}

void GILogic::Update(const Time& time){
	
	//scene_.Update(time);

	deferred_renderer_->Draw(*output_);

}