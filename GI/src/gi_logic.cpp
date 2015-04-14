#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>
#include <renderers.h>

#include <interface.h>

#include <scene.h>
#include <fbx\fbx.h>
#include <Eigen/Geometry>

#include <typeindex>
#include <typeinfo>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::Eigen;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11))
{
	
	// Graphics setup

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

	auto r = graphics_.CreateRenderer<TiledDeferredRenderer, LoadFromFile>({ L"" });

	// Camera setup
	/*
	auto & camera_node = scene_.CreateNode();

	camera_ = camera_node.AddComponent<Camera>(output_->GetRenderTarget());	// The camera will render directly on the backbuffer
	
	camera_->SetFarPlane(1000);
	camera_->SetNearPlane(1);
	camera_->SetFieldOfView(Math::kPi * 0.5f);
	camera_->SetProjectionMode(Camera::ProjectionMode::kPerspective);
	*/
	//Scene import

	//FBXImporter::GetInstance().ImportScene(Application::GetDirectory() + L"Data\\gisponza.fbx", scene_.CreateNode(), graphics_.GetResources());

	// Bounding volume hierarchy rebuild
	
	//scene_.GetBVH().Rebuild();

}

GILogic::~GILogic(){

	output_ = nullptr;

}

void GILogic::Update(const Time & time){
	
	//scene_.Update(time);

}