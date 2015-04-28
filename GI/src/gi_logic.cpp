#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>
#include <renderers.h>

#include <component.h>
#include <range.h>

#include <scene.h>
#include <fbx\fbx.h>
#include <Eigen/Geometry>

#include <typeindex>
#include <typeinfo>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::fbx;
using namespace ::Eigen;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

struct Foo{

	void DoFoo(){}

	void DoBar(){}

};

class MaterialImporter : public IMaterialImporter{

	virtual void OnImportMaterial(MaterialCollection& materials, MeshComponent& mesh){

		// Add the proper material component here


	}

}
;

GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11))
{

	using mtype = Foo*;

	vector<mtype> v;

	Range<vector<mtype>::iterator> ran(v.begin(), v.end());

	auto it = v.begin();

	
	


	// Graphics setup

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

	auto r = graphics_.CreateRenderer<TiledDeferredRenderer, LoadFromFile>({ L"" });

	// Camera setup
	/*
	auto & camera_node = scene_.CreateNode();

	camera_ = camera_node.AddComponent<Camera>(output_->GetRenderTarget());	// The camera will render directly on the backbuffer
	
	camera_->SetFarPlane(1000);
	camera_->SetNearPlane(1);
	camera_->SetFieldOfView(Math::kPi * 0.5f);
	camera_->SetProjectionMode(Camera::ProjectionMode::kPerspective);
	*/

	auto node = scene_.CreateNode(L"root", Translation3f(Vector3f::Zero()), Quaternionf::Identity(), AlignedScaling3f(Vector3f::Ones()));

	FbxImporter fbx_importer(MaterialImporter(),
							 graphics_.GetResources());

	fbx_importer.ImportScene(Application::GetDirectory() + L"Data\\gisponza.fbx", 
							 *node);

	//Scene import

	//FBXImporter::GetInstance().ImportScene(Application::GetDirectory() + L"Data\\gisponza.fbx", scene_.CreateNode(), graphics_.GetResources());

	// Bounding volume hierarchy rebuild
	
	//scene_.GetBVH().Rebuild();

}

GILogic::~GILogic(){

	output_ = nullptr;

}

void GILogic::Update(const Time & time){
	
	//scene_.Update(time);

}