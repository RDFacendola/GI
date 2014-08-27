#include "gi_logic.h"

#include <string>

#include <resources.h>

#include <dx11/dx11graphics.h>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	graphics_(DX11Graphics::GetInstance()){

	/////////////////////////////////////

	auto & r = graphics_.GetManager();

	size_t s1, s2;

	{
		auto p = r.Load<Mesh>(L"crysponza.fbx");

		s1 = r.GetSize();

	}
	
	//Texture now should have been destroyed

	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	auto c = output_->GetAntialisingMode();

	output_->Commit();
	
}