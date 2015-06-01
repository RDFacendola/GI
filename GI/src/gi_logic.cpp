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

#include "..\include\material_importer.h"

using namespace ::std;
using namespace ::gi;
using namespace ::gi_lib;
using namespace ::gi_lib::fbx;
using namespace ::Eigen;

/// \brief Title of the main window.
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

/// \brief Size of the domain (for each edge).
const float kDomainSize = 2000.0f;

/// \brief Number of times the domain is split along each axis.
const unsigned int kDomainSubdivisions = 1;

TransformComponent* camera_transform;

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

	camera_transform = scene_.CreateNode(L"MainCamera",
										 Translation3f(Vector3f(0.0f, 300.0f, 0.0f)),
										 Quaternionf::Identity(),
										 AlignedScaling3f(Vector3f::Ones()));

	auto camera = camera_transform->AddComponent<CameraComponent>();

	camera->SetProjectionType(ProjectionType::Perspective);
	camera->SetMinimumDistance(1.0f);
	camera->SetMaximumDistance(10000.0f);
	camera->SetFieldOfView(Math::DegToRad(90.0f));

	scene_.SetMainCamera(camera);

	camera_transform->SetRotation(Quaternionf(AngleAxisf(Math::DegToRad(90.0f),
														 Vector3f(0.0f, 1.0f, 0.0f))));

	// Scene import

	auto node = scene_.CreateNode(L"root", 
								  Translation3f(Vector3f(0.0f, 0.0f, 0.0f)),
								  Quaternionf::Identity(), 
								  AlignedScaling3f(Vector3f::Ones() * 3.0f));

	auto& resources = graphics_.GetResources();

	MaterialImporter material_importer(resources);

	FbxImporter fbx_importer(material_importer,
							 resources);
	
	fbx_importer.ImportScene(to_string(Application::GetDirectory()) + "Data\\oldsponza.fbx", 
							 *node);
	
}

GILogic::~GILogic(){

	output_ = nullptr;

}

void GILogic::Update(const Time& time){
	
	auto rotation = camera_transform->GetRotation();

	rotation *= Quaternionf(Eigen::AngleAxisf(time.GetDeltaSeconds()* 0.2f, 
											  Vector3f(0.0f, 1.0f, 0.0f)));

	auto angle = std::sinf(time.GetTotalSeconds() * 0.15f);

	camera_transform->SetTranslation(Translation3f(angle * 1500.0f, 300.0f, 0.0f));

	camera_transform->SetRotation(rotation);

	deferred_renderer_->Draw(*output_);

}