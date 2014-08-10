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

auto lambda = Window::TOnResized::TListener([](Window & win, unsigned int w, unsigned int h){

	wstringstream wss;

	wss << std::to_wstring(w) << L" x " << std::to_wstring(h);

	SetWindowText(win.GetHandle(), wss.str().c_str() );

});

GILogic::GILogic(){

	SetTitle(kWindowTitle);

	Show();
	
	OnResized() << lambda;

}

GILogic::~GILogic(){

}

void GILogic::Update(const Timer::Time & time){

	
}