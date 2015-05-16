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

/// \brief Number of times the domain is splitted along each axis.
const unsigned int kDomainSubdivisions = 3;

class MaterialImporter : public IMaterialImporter{

	virtual void OnImportMaterial(MaterialCollection& materials, MeshComponent& mesh){

		// Add a renderer component for the deferred renderer.
		mesh.AddComponent<DeferredRendererComponent>(mesh);

		// Set the proper materials



	}

}
;

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

	FbxImporter fbx_importer(MaterialImporter(),
							 graphics_.GetResources());
	
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