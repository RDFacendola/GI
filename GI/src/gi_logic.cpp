#include "gi_logic.h"

#include <string>

#include <texture.h>

#include <dx11/dx11factory.h>
#include <dx11/dx11graphics.h>
#include <dx11/dx11resources.h>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	/////////////////////////////////////

	auto & r = factory_.GetResources();

	auto p = r.Load<Texture2D>(L"femalehead4K.dds");

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