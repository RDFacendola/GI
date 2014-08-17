#include "gi_logic.h"

#include <string>

#include "application.h"
#include "exceptions.h"
#include "timer.h"
#include "system.h"
#include "scene.h"
#include "graphics.h"
#include "dx11graphics.h"
#include "dx11resources.h"

using namespace ::std;
using namespace ::gi_lib;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	/////////////////////////////////////

	DX11Resources res;

	auto k = res.Get<ITexture3D>();

	

	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto ap = factory_.GetAdapterProfile();

	graphics_ = factory_.CreateGraphics(*this);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Timer::Time & time){
	
	graphics_->Commit();
	
}