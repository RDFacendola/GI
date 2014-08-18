#include "gi_logic.h"

#include <string>

#include "dx11graphics.h"
#include "dx11resources.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	/////////////////////////////////////



	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto ap = factory_.GetAdapterProfile();

	graphics_ = factory_.CreateGraphics(*this);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	graphics_->Commit();
	
}