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
								kDomainSubdivisions * Vector3i::Ones())),
input_(nullptr)
{}

GILogic::~GILogic(){

	output_ = nullptr;

}

void GILogic::Initialize(Window& window){

	// Graphics setup

	window.SetTitle(kWindowTitle);

	window.Show();
	
	// Create the output window

	output_ = graphics_.CreateOutput(window, 
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
	
	auto& app = Application::GetInstance();

	fbx_importer.ImportScene(to_string(app.GetDirectory()) + "Data\\oldsponza.fbx",
							 *node);

	// Input

	input_ = &window.GetInput();

}

void GILogic::Update(const Time & time){

	// Strafing

	auto& kb = input_->GetKeyboardStatus();

	Vector3f direction = Vector3f::Zero();

	if (kb.IsDown(17)){

		// Up
		direction += camera_transform->GetForward();

	}
	
	if (kb.IsDown(31)){

		// Down
		direction -= camera_transform->GetForward();

	}

	if (kb.IsDown(32)){

		// Right
		direction += camera_transform->GetRight();

	}

	if (kb.IsDown(30)){

		// Left
		direction -= camera_transform->GetRight();


	}

	if (direction.squaredNorm() > 0){

		direction.normalize();

		Vector3f translation = camera_transform->GetTranslation().vector();
		
		translation += direction * time.GetDeltaSeconds() * 500.0f;
		
		camera_transform->SetTranslation(Translation3f(translation));

	}

	// Rotation

	auto& mb = input_->GetMouseStatus();

	if (mb.IsDown(0)){

		auto movement = mb.GetMovement() * time.GetDeltaSeconds();

		auto hrotation = Quaternionf(AngleAxisf(movement(0) * 6.28f, camera_transform->GetUp()));

		auto vrotation = Quaternionf(AngleAxisf(movement(1) * -1.0f, camera_transform->GetRight()));

		auto rotation = camera_transform->GetRotation();

		camera_transform->SetRotation(rotation * hrotation * vrotation);

	}

	deferred_renderer_->Draw(*output_);

}