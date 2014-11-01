#include "..\include\components.h"

#include "..\include\scene.h"
#include "..\include\gimath.h"

using namespace gi_lib;
using namespace std;

///////////////////// NODE COMPONENT ///////////////////

NodeComponent::NodeComponent(SceneNode & node) :
	node_(node),
	enabled_(true){}

NodeComponent::~NodeComponent(){}

/////////////////////// RENDERER ///////////////////////////////////////

Renderer::Renderer(SceneNode & node) :
NodeComponent(node){

	

}

Renderer::~Renderer(){

}

void Renderer::PostUpdate(const Time & time){

	// Update the bounds of the geometry
	auto & node = GetNode();

	auto geometry = node.GetComponent<StaticGeometry>();
	
	bounds_ = geometry->GetMesh()
					  ->GetBounds()
					   .Transformed(node.GetWorldTransform());
	
}

/////////////////////// CAMERA /////////////////////////////////////////

Camera::Camera(SceneNode & node, shared_ptr<RenderTarget> target) :
NodeComponent(node),
target_(target){

	if (!target_){

		throw RuntimeException(L"Render target must not be null!");

	}

	projection_mode_ = ProjectionMode::kPerspective;
	clear_mode_ = ClearMode::kColor;

	viewport_.position = Vector2f::Zero();
	viewport_.extents = Vector2f::Ones();

	aspect_ratio_ = target_->GetAspectRatio();

	near_plane_ = 1.0f;
	far_plane_ = 1000.0f;

	clear_color_ = Color{ { 1.0f, 0.0f, 0.0f, 0.0f } };

	field_of_view_ = Math::DegToRad(60.0f);
	ortho_size_ = 1.0f;

}

void Camera::Update(const Time &){

	// The camera is likely to move every frame, so the view matrix must be always recomputed.
	UpdateViewMatrix(true);

	// The only external factor is the aspect ratio: if it changes the projection matrix must be recomputed.

	auto aspect_ratio = target_->GetAspectRatio();

	if (!Math::Equal(aspect_ratio, aspect_ratio_, 0.01f)){

		aspect_ratio_ = aspect_ratio;

		UpdateProjectionMatrix(true);

	}

}

Affine3f Camera::GetViewMatrix(){

	UpdateViewMatrix();

	return view_matrix_;

}

Projective3f Camera::GetProjectionMatrix(){

	UpdateProjectionMatrix();

	return proj_matrix_;

}

void Camera::UpdateProjectionMatrix(bool force){

	if (!projection_dirty_ && 
		!force){
		
		return;

	}

	// TODO: Update the projection matrix

	projection_dirty_ = false;

}

void Camera::UpdateViewMatrix(bool force){

	if (!view_dirty_ &&
		!force){

		return;

	}

	// TODO: Update the view matrix

	view_dirty_ = false;

}