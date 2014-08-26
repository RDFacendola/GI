#include "gi_logic.h"

#include <string>

#include <texture.h>

#include <dx11/dx11factory.h>
#include <dx11/dx11graphics.h>
#include <dx11/dx11resource_manager.h>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	/////////////////////////////////////

	auto & r = factory_.GetResourceManager();

	size_t s1, s2;

	{
		auto p = r.Load<Texture2D>(L"femalehead4K.dds");

		s1 = r.GetSize();

	}
	
	//Texture now should have been destroyed

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