#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>

#include <scene.h>
#include <Eigen/Geometry>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::Eigen;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11))
{


	/////////////////////////////////////

	auto & r = graphics_.GetManager();
	
	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	


	auto & n = Scene();

	auto & f = n.CreateNode(L"First level", Affine3f(Translation3f(Vector3f(10.0f, 0.0f, 0.0f))));

	auto & s = n.CreateNode(L"Second level", Affine3f(AngleAxisf(3.14159f, Vector3f::UnitZ())));

	s.SetParent(f);

	n.Update(time);

	auto wt = s.GetTransform().GetWorldTransform();

	auto v = Vector3f(0.0f, 0.0f, 0.0f);

	auto t = wt * v;

	auto c = output_->GetAntialisingMode();

	output_->Commit();
	
}