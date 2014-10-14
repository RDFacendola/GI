#include "..\include\gi_logic.h"

#include <string>

#include <resources.h>

#include <scene.h>
#include <fbx\fbx.h>
#include <Eigen/Geometry>

using namespace ::std;
using namespace ::gi_lib;
using namespace ::Eigen;

const wstring kWindowTitle = L"Global Illumination - Raffaele D. Facendola";

class Foo : public NodeComponent{

public:

	Foo(SceneNode & node) :
		NodeComponent(node){}

protected:

	virtual void Update(const Time & time) override {};
	
};

class Bar : public Foo{

public:

	Bar(SceneNode & node) :
		Foo(node){}

};



GILogic::GILogic() :
graphics_(Graphics::GetAPI(API::DIRECTX_11)),
scene_(make_unique<Scene>())
{

	/// TESTING STUFFS

	auto & h = scene_->CreateNode();

	h.AddComponent<Foo>();
	h.AddComponent<Bar>();

	auto foos = h.GetComponents<Foo>().size();
	auto bars = h.GetComponents<Bar>().size();
	
	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

	/////////////////////////////////////

	auto & camera = scene_->CreateNode();

	camera.AddComponent<Camera>(output_->GetRenderTarget());

	auto & node = scene_->CreateNode();

	auto & manager = graphics_.GetManager();

	//FBXImporter::GetInstance().ImportScene(Application::GetInstance().GetDirectory() + L"Data\\gisponza.fbx", node, graphics_.GetManager());

	auto s = manager.GetSize();

}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	scene_->Update(time);

	output_->Commit();
	
}