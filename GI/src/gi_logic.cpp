#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>

#include <scene.h>
#include <fbx\fbx.h>
#include <Eigen/Geometry>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::Eigen;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11)),
scene_(make_unique<Scene>())
{
	
	/////////////////////////////////////

	auto & r = graphics_.GetManager();

	auto & node = scene_->CreateNode();

	FBXImporter::GetInstance().ImportScene(Application::GetInstance().GetDirectory() + L"Data\\crysponza_tri.fbx", node, graphics_.GetManager());

	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	scene_->Update(time);

	output_->Commit();
	
}