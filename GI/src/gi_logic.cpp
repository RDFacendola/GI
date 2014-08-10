#include "gi_logic.h"

#include <string>

#include "application.h"
#include "exceptions.h"
#include "timer.h"
#include "system.h"
#include "scene.h"

using namespace ::std;
using namespace ::gi_lib;

const unsigned int kWindowWidth = 1280;
const unsigned int kWindowHeight = 768;
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

SceneObject so;

class FooComponent : public SceneObject::Component{

public:

	void do_foo(){}

	void do_cfoo() const{}

protected:

	virtual void Update(const Timer::Time & time){

		//Do nothing

	}

};

class BarComponent : public SceneObject::Component{

public:

	void do_bar(){}

	void do_cbar() const{}

protected:

	virtual void Update(const Timer::Time & time){

		//Do nothing

	}

};

GILogic::GILogic(){

	SetTitle(kWindowTitle);

	Show();

	counter = 0;
	
}

GILogic::~GILogic(){

}

void GILogic::Update(const Timer::Time & time){

	

	if (time.GetTotalSeconds() >= 10){

		wstringstream message;

		message << std::to_wstring(counter);

		SetTitle(message.str());

		Sleep(10);

	}else{
		
		++counter;

	}


}

LRESULT GILogic::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	if (message_id == WM_CLOSE){

		gi_lib::Application::GetInstance()
							.DisposeWindow(GetHandle());
		
	}

	//Default behavior
	return DefWindowProc(GetHandle(), message_id, wparameter, lparameter);

}