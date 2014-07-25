#include "gi_logic.h"

#include "exceptions.h"
#include "raii.h"
#include "dx11graphics.h"

const unsigned int kWindowWidth = 1280;
const unsigned int kWindowHeight = 768;
const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic(){}

void GILogic::Initialize(HWND window_handle){

	//Initialize DirectX11
	graphics_ = new DX11Graphics();

	GRAPHIC_MODE graphic_mode;

	graphic_mode.video.horizontal_resolution = 1920;
	graphic_mode.video.vertical_resolution = 1080;
	graphic_mode.video.refresh_rate_Hz = 60;
	graphic_mode.antialiasing = ANTIALIASING_MODE::NONE;
	graphic_mode.windowed = true;
	graphic_mode.vsync = false;
	
	graphics_->CreateOrDie(window_handle,
						   graphic_mode);
	
}

void GILogic::Update(HWND window_handle, const APPLICATION_TIME & time){

	//End of frame
	graphics_->Present();
	
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