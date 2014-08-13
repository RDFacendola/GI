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

class ComponentFoo : public SceneObject::Component{

public:

	ComponentFoo(int v): value(v){}

	int value = 0;

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

	SceneObject so;

	ComponentFoo& x = so.AddComponent<ComponentFoo>(7);
	
	x.value = 47;

	ComponentFoo zzz(1);

	zzz = x;

	so.RemoveComponent(zzz);

	ComponentFoo& y = so.GetComponent<ComponentFoo>();

	if (&x == &y){

		bool yay;

		yay = true;

	}

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