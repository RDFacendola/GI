#include "..\include\gi_logic.h"

#include <string>
#include <typeindex>
#include <typeinfo>

#include "fbx/fbx.h"

#include "Eigen/Geometry"

#include "resources.h"
#include "render_target.h"
#include "deferred_renderer.h"
#include "uniform_tree.h"
#include "component.h"
#include "range.h"
#include "scene.h"
#include "texture.h"
#include "material_importer.h"
#include "fly_camera_component.h"
#include "light_component.h"

#include "texture.h"

#include "gpgpu.h"

#include <Windows.h>

using namespace ::std;
using namespace ::gi;
using namespace ::gi_lib;
using namespace ::gi_lib::fbx;
using namespace ::Eigen;

/// \brief Title of the main window.
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

/// \brief Size of the domain (for each edge).
const float kDomainSize = 5600.0f;

/// \brief Number of times the domain is split along each axis.
const unsigned int kDomainSubdivisions = 2;

FlyCameraComponent* fly_camera;

Window* g_window;

GILogic::GILogic() :
	graphics_(Graphics::GetAPI(API::DIRECTX_11)),
	input_(nullptr){

	scene_ = make_unique<Scene>(make_unique<UniformTree>(AABB{ Vector3f::Zero(),							// Mesh hierarchy
															   kDomainSize * Vector3f::Ones() },
														 kDomainSubdivisions * Vector3i::Ones()),
								make_unique<UniformTree>(AABB{ Vector3f::Zero(),							// Light hierarchy
															   kDomainSize * Vector3f::Ones() },
														 kDomainSubdivisions * Vector3i::Ones()));

}

GILogic::~GILogic(){

	scene_ = nullptr;

	output_ = nullptr;

	deferred_renderer_ = nullptr;
	
}

void GILogic::Initialize(Window& window){

	// Graphics setup
	g_window = &window;

	window.SetTitle(kWindowTitle);

	window.Show();
	
	// Create the output window

	output_ = graphics_.CreateOutput(window, 
									 graphics_.GetAdapterProfile().video_modes[0]);

	// Create the renderers

	deferred_renderer_ = std::move(graphics_.CreateRenderer<TiledDeferredRenderer>(*scene_));

	// Camera setup

	auto camera_transform = scene_->CreateNode(L"MainCamera",
											   Translation3f(0.0f, 300.0f, 0.0f),
											   Quaternionf::Identity(),
											   AlignedScaling3f(1.0f, 1.0f, 1.0f));

	auto camera = camera_transform->AddComponent<CameraComponent>();

	camera->SetProjectionType(ProjectionType::Perspective);
	camera->SetMinimumDistance(1.0f);
	camera->SetMaximumDistance(10000.0f);
	camera->SetFieldOfView(Math::DegToRad(90.0f));

	scene_->SetMainCamera(camera);

	input_ = &window.GetInput();

	fly_camera = camera->AddComponent<FlyCameraComponent>(*input_);

	// Scene import

	auto root = scene_->CreateNode(L"root", 
								   Translation3f(Vector3f(0.0f, 0.0f, 0.0f)),
								   Quaternionf::Identity(), 
								   AlignedScaling3f(Vector3f::Ones() * 3.0f));

	auto& resources = graphics_.GetResources();

	MaterialImporter material_importer(resources);

	FbxImporter fbx_importer(material_importer,
							 resources);
	
	auto& app = Application::GetInstance();

	fbx_importer.ImportScene(to_string(app.GetDirectory()) + "Data\\assets\\oldsponza.fbx",
							 *root);

	
	// Lights setup 

	// Point light

	auto light_transform = scene_->CreateNode(L"PointLight",
											  Translation3f(1500.0f, 250.0f, 0.0f),
											  Quaternionf::Identity(),
											  AlignedScaling3f(1.0f, 1.0f, 1.0f));

	auto point_light = light_transform->AddComponent<PointLightComponent>(Color(8.f, 12.f, 16.f, 1.0f), 100.0f);

	light_transform->SetParent(root);

	// Directional light

	light_transform = scene_->CreateNode(L"DirectionalLight",
										 Translation3f(0.0f, 0.0f, 0.0f),
										 Quaternionf(AngleAxisf(Math::kDegToRad * 45.0f, Vector3f(0.0f, 0.0f, 1.0f))),
										 AlignedScaling3f(1.0f, 1.0f, 1.0f));

	auto directional_light = light_transform->AddComponent<DirectionalLightComponent>(Color(1.78f, 1.32f, 0.68f, 1.0f));

	light_transform->SetParent(root);
	
}

void GILogic::Update(const Time & time){

	fly_camera->Update(time);
	
	auto next_frame = deferred_renderer_->Draw(output_->GetVideoMode().horizontal_resolution * 1.0f,
											   output_->GetVideoMode().vertical_resolution * 1.0f);

	output_->Display(next_frame);

}