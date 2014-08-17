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
using namespace ::gi_lib::dx11;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

class Foo{
public:

	void foo(){}

};

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	/////////////////////////////////////

	ID3D11Device * dev;

	DX11Resources res(*dev);

	auto p = std::make_shared<Foo>();

	auto h = ::Handle<Foo>(p);

	h->foo();

	auto ahah = ::Handle<Texture2D>();

	auto k = res.Get(ahah);

	

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