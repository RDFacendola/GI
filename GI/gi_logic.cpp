#include "gi_logic.h"

#include "exceptions.h"
#include "raii.h"
#include "dx11graphics.h"

const unsigned int kWindowWidth = 1280;
const unsigned int kWindowHeight = 768;
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic(){

	factory_ = nullptr;
	graphics_ = nullptr;
	
}

void GILogic::Initialize(Window & window){

	//Setup the window
	auto window_handle = window.GetWindowHandle();

	SetWindowText(window_handle, kWindowTitle.c_str());
	
	ShowWindow(window_handle, SW_SHOWDEFAULT);

	//Initialize DirectX11
	factory_ = new DX11Factory();

	auto p = factory_->GetProfile();

	graphics_ = factory_->Create(window);

	//graphics_->EnableVSync(false);

}

void GILogic::Destroy(){

	//Destroy everything
	if (graphics_ != nullptr){

		delete graphics_;

	}

	if (factory_ != nullptr){

		delete factory_;

	}

}

void GILogic::Update(HWND window_handle, const APPLICATION_TIME & time){

	//Next frame

	graphics_->NextFrame();
	
}

LRESULT GILogic::ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

	//Manage the message

	//Default behavior
	return DefWindowProc(window_handle, message_id, wparameter, lparameter);

}