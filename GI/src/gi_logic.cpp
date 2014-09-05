#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>

#include <scene.h>

using namespace ::std;
using namespace ::gi_lib;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

GILogic::GILogic():
	graphics_(Graphics::GetAPI(API::DIRECTX_11))
{


	/////////////////////////////////////

	auto & r = graphics_.GetManager();

	auto ahah = r.Load<Mesh, Mesh::LoadMode::kFromFBX>({ L"crysponza.fbx", L"crysponza_00" });
	//auto t = r.Load<Texture2D, Texture2D::LoadMode::kFromDDS>({ L"Data\\femalehead4K.dds" });

	SceneNode n = SceneNode(L"My node", Affine3f::Identity(), {});

	auto & q = n.GetTransform();

	

	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	auto c = output_->GetAntialisingMode();

	output_->Commit();
	
}