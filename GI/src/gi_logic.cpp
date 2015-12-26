#include "..\include\gi_logic.h"

#include <string>
#include <typeindex>
#include <typeinfo>

#include "fbx/fbx.h"
#include "wavefront/wavefront_obj.h"

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
	input_(nullptr),
	paused_(false){

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

	deferred_renderer_ = std::move(graphics_.CreateRenderer<DeferredRenderer>(*scene_));

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

	auto& app = Application::GetInstance();

	MtlMaterialImporter material_importer(resources);

	wavefront::ObjImporter obj_importer(resources);
		
	obj_importer.ImportScene(app.GetDirectory() + L"Data\\assets\\Sponza\\SponzaNoFlag.obj",
							 *root,
							 material_importer);
	
	auto skybox = scene_->CreateNode(L"skybox",
									 Translation3f(Vector3f::Zero()),
									 Quaternionf::Identity(),
									 AlignedScaling3f(Vector3f::Ones() * 500.0f));

	obj_importer.ImportScene(app.GetDirectory() + L"Data\\assets\\Skybox\\Skybox.obj",
							 *skybox,
							 material_importer);

	// Lights setup 

	SetupLights(*scene_, 
				obj_importer.ImportMesh(app.GetDirectory() + L"Data\\assets\\Light\\Sphere.obj", "Icosphere"));
	
}

void GILogic::SetupLights(Scene& scene, ObjectPtr<IStaticMesh> point_light_mesh) {

	// Disable light's shadowcasting
	
	point_light_mesh->SetFlags(MeshFlags::kNone);

	// Point lights

	struct PerPointLight {

		Vector4f gColor;			// Point light emissive color

	};

	auto& resources = graphics_.GetResources();

	auto base_material = resources.Load<DeferredRendererMaterial, DeferredRendererMaterial::CompileFromFile>({ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\mat_emissive.hlsl" });

	static std::vector<Color> kLightColors{ Color(5.f, 5.f, 5.f, 1.f),
											Color(5.f, 5.f, 5.f, 1.f),
											Color(5.f, 5.f, 5.f, 1.f),
											/*Color(5.f, 5.f, 5.f, 1.f),
											Color(25.f, 10.f, 25.f, 1.f),
											Color(10.f, 25.f, 25.f, 1.f)*/};

	static float kPointLightRadius = 250.0f;

	for (auto&& light_color : kLightColors) {

		auto light_node = scene_->CreateNode(L"PointLight",
											 Translation3f::Identity(),
											 Quaternionf::Identity(),
											 AlignedScaling3f(kPointLightRadius, kPointLightRadius, kPointLightRadius));

		// Point light setup

		auto light_component = light_node->AddComponent<PointLightComponent>(light_color, kPointLightRadius);
		
		light_component->SetCutoff(0.001f);
		light_component->EnableShadow(true);
		light_component->SetShadowMapSize(Vector2i(1024, 512));

		point_lights.push_back(light_node);
		
		// Light mesh

		auto mesh_component = light_node->AddComponent<MeshComponent>(point_light_mesh);

		// Light material

		auto deferred_component = light_node->AddComponent<AspectComponent<DeferredRendererMaterial>>(*mesh_component);

		auto material_instance = base_material->Instantiate();

		deferred_component->SetMaterial(0, material_instance);

		auto per_point_light = resources.Load<IStructuredBuffer, IStructuredBuffer::FromSize>({ sizeof(PerPointLight) });

		auto& buffer = *per_point_light->Lock<PerPointLight>();

		buffer.gColor = light_color.ToVector4f();

		per_point_light->Unlock();

		material_instance->GetMaterial()->SetInput("PerMaterial", ObjectPtr<IStructuredBuffer>(per_point_light));

	}
	
	// Sky contribution
	{
		
		return;

		auto light_node = scene_->CreateNode(L"DirectionalLight",
											 Translation3f(0.0f, 0.0f, 0.0f),
											 Quaternionf::Identity(),
											 AlignedScaling3f(1.0f, 1.0f, 1.0f));

		auto light_component = light_node->AddComponent<DirectionalLightComponent>(Color(1.1f, 1.1f, 1.1f, 1.0f));

		light_component->EnableShadow(true);
		light_component->SetShadowMapSize(Vector2i(1024, 1024));

		directional_lights.push_back(light_node);

	}
	

}

void GILogic::Update(const Time & time){

	fly_camera->Update(time);
	
	if (input_->GetKeyboardStatus().IsPressed((25))){		// "P"

		paused_ = !paused_;

	}

	if (!paused_) {

		static const float xRadius = 2000.0f;
		static const float yRadius = 750.0f;
		static const float zRadius = 400.0f;
	
		static const float angular_speed = Math::kPi / 16.0f;
		static const float oscillation_speed = Math::kPi / 6.0f;

		static float game_time = 0.0f;

		game_time += time.GetDeltaSeconds();

		float light_index = 0.0f;
		float light_angle;

		for (auto&& point_light : point_lights) {

			light_angle = light_index / point_lights.size();
			light_angle *= Math::kPi * 2.0f;

 			point_light->SetTranslation(Translation3f(std::cosf(light_angle + game_time * angular_speed) * xRadius - 150.f,
 													  std::cosf(light_angle + game_time * oscillation_speed) * yRadius + 1000.f,
 													  std::sinf(light_angle + game_time * angular_speed) * zRadius - 150.f));

			++light_index;

		}

		light_index = 0.0f;

		for (auto&&directional_light : directional_lights) {

			light_angle = light_index / directional_lights.size();
			light_angle *= Math::kPi * 2.0f;
			
			directional_light->SetRotation(Quaternionf(AngleAxisf(Math::DegToRad(90.f), Vector3f(1.f, 0.f, 0.f))) *
										   Quaternionf(AngleAxisf(game_time * angular_speed, Vector3f(0.f, 1.f, 0.f))));
			
			auto forward = directional_light->GetForward();

			++light_index;

		}

	}

	auto next_frame = deferred_renderer_->Draw(output_->GetVideoMode().horizontal_resolution * 1.0f,
											   output_->GetVideoMode().vertical_resolution * 1.0f);

	output_->Display(next_frame);

}