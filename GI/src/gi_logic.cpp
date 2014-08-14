#include "gi_logic.h"

#include <string>

#include "application.h"
#include "exceptions.h"
#include "timer.h"
#include "system.h"
#include "scene.h"
#include "graphics.h"
#include "dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;

const unsigned int kWindowWidth = 1280;
const unsigned int kWindowHeight = 768;
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

class ComponentFoo : public Component{

public:

	ComponentFoo() : value(0){}

	ComponentFoo(int v): value(v){}

	int value;

protected:

	virtual void Update(const Timer::Time & time){

		//Do nothing

	}

};

class Bar{

public:

};

GILogic::GILogic():
	factory_(DX11Factory::GetInstance()){

	SceneObject so(L"Scene!", { L"Caio", L"Cmoe", L"Tsai?" });

	auto a = so.HasTag(L"Scene!");
	auto b = so.HasTag(L"Cmoe");

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