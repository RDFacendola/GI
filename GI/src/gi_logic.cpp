#include "gi_logic.h"

#include <string>

#include "application.h"
#include "exceptions.h"
#include "timer.h"
#include "system.h"

using namespace ::std;

const unsigned int kWindowWidth = 1280;
const unsigned int kWindowHeight = 768;
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

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