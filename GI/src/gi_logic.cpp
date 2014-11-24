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
scene_(Scene::GetInstance())
{
	
	/// TESTING STUFFS

	/////////////////////////////////////

	SetTitle(kWindowTitle);

	Show();
	
	auto p = graphics_.GetAdapterProfile();

	output_ = graphics_.CreateOutput(*this, p.video_modes[0]);

	/////////////////////////////////////
	
	SceneNodeBuilder camera_builder = scene_.GetRoot().GetBuilder();

	auto & camera_node = camera_builder.Build();
		
	camera_ = camera_node.AddComponent<Camera>(output_->GetRenderTarget());
	
	camera_->SetFarPlane(1000);
	camera_->SetNearPlane(1);
	camera_->SetFieldOfView(Math::kPi * 0.5f);
	camera_->SetProjectionMode(Camera::ProjectionMode::kPerspective);

	//camera_node.SetRotation(Quaternionf(AngleAxisf(Math::kPi * 0.5f, Vector3f::Identity())));
	//camera_node.SetPosition(Translation3f(Vector3f(0.0f, 0.0f, 1000.0f)));

	auto & manager = graphics_.GetManager();

	//FBXImporter::GetInstance().ImportScene(Application::GetInstance().GetDirectory() + L"Data\\gisponza.fbx", scene_.GetRoot(), graphics_.GetManager());

	auto s = manager.GetSize();

	////////////////////////////////////

	scene_.GetBVH().Rebuild();
		
}

GILogic::~GILogic(){

}

void GILogic::Update(const Time & time){
	
	scene_.Update(time);

	auto f = camera_->GetViewFrustum();

	output_->Commit();
	
}