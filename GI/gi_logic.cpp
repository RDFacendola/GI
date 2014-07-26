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

void GILogic::Initialize(HWND window_handle){

	//Initialize DirectX11
	factory_ = new DX11Factory();

	auto profile = factory_->GetProfile();
	
	graphics_ = factory_->Create(window_handle);

	graphics_->EnableVSync(false);
	graphics_->EnableFullscreen(true);

}

void GILogic::Update(HWND window_handle, const APPLICATION_TIME & time){

	//Next frame
	graphics_->NextFrame();
	
}

LRESULT GILogic::ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter, const APPLICATION_TIME & time){

	//Manage the message

	//Default behavior
	return DefWindowProc(window_handle, message_id, wparameter, lparameter);

}


void GILogic::Destroy(){

	if (graphics_ != nullptr){

		delete graphics_;

	}

	if (factory_ != nullptr){

		delete factory_;

	}

}

bool GILogic::IsRunning() const{

	return is_running_;

}

void GILogic::Exit(){

	is_running_ = false;

}

unsigned int GILogic::GetWindowWidth() const{

	return kWindowWidth;

}

unsigned int GILogic::GetWindowHeight() const{

	return kWindowHeight;

}

wstring GILogic::GetWindowTitle() const{

	return kWindowTitle;
	
}