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

	Affine3f T(Translation3f(10, 20, 30));
	Affine3f S(AngleAxisf(3.1459f, Vector3f::UnitX()));

	auto TS = T*S;
	auto ST = S*T;

	auto r0 = TS * Vector3f::Zero();
	auto r1 = ST * Vector3f::Zero();

	auto & r = graphics_.GetManager();

	auto & node = scene_->CreateNode();

	auto & manager = graphics_.GetManager();

	FBXImporter::GetInstance().ImportScene(Application::GetInstance().GetDirectory() + L"Data\\gisponza.fbx", node, graphics_.GetManager());

	auto s = manager.GetSize();

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