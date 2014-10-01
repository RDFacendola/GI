#include "..\include\components.h"

#include "..\include\scene.h"
#include "..\include\gimath.h"

using namespace gi_lib;
using namespace std;

///////////////////// NODE COMPONENT ///////////////////

NodeComponent::NodeComponent() :
	owner_(nullptr),
	enabled_(true){}

NodeComponent::~NodeComponent(){}

//////////////////// TRANSFORM ////////////////////////

Transform::Transform(const Affine3f & local_transform) :
	local_transform_(local_transform),
	world_transform_(Affine3f::Identity()),
	parent_(nullptr){
	
}

Transform::Transform(Transform && other) :
	local_transform_(std::move(other.local_transform_)),
	world_transform_(std::move(other.world_transform_)),
	parent_(other.parent_),
	children_(std::move(other.children_)){

	//Remove "other" from the parent and add "this"
	
	if (parent_ != nullptr){

		parent_->RemoveChild(other);

		parent_->AddChild(*this);

	}
	
}

Transform::~Transform(){

	for (auto & child : children_){

		child->parent_ = nullptr;
		GetOwner().GetScene().SetRoot(child->GetOwner());

	}

	children_.clear();

	if (parent_ != nullptr){

		parent_->RemoveChild(*this);

	}
	
}

void Transform::SetParent(Transform & parent){

	if (parent_ != nullptr){

		parent_->RemoveChild(*this);

	}
	
	parent.AddChild(*this);
	
	parent_ = &parent;

}

Transform& Transform::GetParent(){

	if (parent_ == nullptr){

		throw RuntimeException(L"Transform::GetParent() failed: The node is a root");

	}

	return *parent_;

}

const Transform& Transform::GetParent() const{

	if (parent_ == nullptr){

		throw RuntimeException(L"Transform::GetParent() failed: The node is a root");

	}

	return *parent_;

}

void Transform::Update(const Time &){

	// TODO: Implementing some "dirty" flag will prevent useless matrix products that do not change.
	// TODO: It may be useful using some "static" flag somewhere...

	if (parent_ != nullptr)
	{

		// Local transform first, then world transform
		world_transform_ = parent_->GetWorldTransform() * local_transform_;

	}
	else
	{

		world_transform_ = local_transform_;

	}
	
}

void Transform::UpdateOwner(const Time & time){

	GetOwner().Update(time);

}

void Transform::AddChild(Transform & child){

	children_.push_back(&child);

}

void Transform::RemoveChild(Transform & child){
	
	children_.erase(std::remove(children_.begin(),
								children_.end(),
								&child),
					children_.end());

}

/////////////////////// RENDERER ///////////////////////////////////////

/////////////////////// CAMERA /////////////////////////////////////////

Camera::Camera(shared_ptr<RenderTarget> target) :
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

	//

	projection_dirty_ = false;

}

void Camera::UpdateViewMatrix(bool force){

	if (!view_dirty_ &&
		!force){

		return;

	}

	//

	view_dirty_ = false;

}