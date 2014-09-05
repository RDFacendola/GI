#include "..\include\components.h"

#include "..\include\scene.h"

using namespace gi_lib;
using namespace std;

///////////////////// NODE COMPONENT ///////////////////

NodeComponent::NodeComponent() :
	owner_(nullptr),
	enabled_(true){}

NodeComponent::~NodeComponent(){}

//////////////////// TRANSFORM ////////////////////////

Transform::Transform() : Transform(Affine3f::Identity()){}

Transform::~Transform(){

	auto parent = parent_;

	Detach();

	if (parent != nullptr){

		for (auto child : children_){

			child->parent_ = nullptr;
			parent->AddChild(*child);

		}

	}
	else
	{

		for (auto child : children_){

			child->parent_ = nullptr;

		}

	}

	children_.clear();

}

Transform::Transform(const Affine3f & local_transform) :
	local_transform_(local_transform),
	world_transform_(Affine3f::Identity()),
	parent_(nullptr){}

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

Transform & Transform::AddChild(Transform & child){

	child.Detach();

	children_.push_back(&child);

	child.parent_ = this;

	return *this;
	
}

void Transform::Detach(){

	if (parent_ != nullptr){

		parent_->RemoveChild(*this);
		parent_ = nullptr;

	}

}

Maybe<Transform&> Transform::GetParent(){

	if (parent_ != nullptr){

		return Maybe<Transform&>(*parent_);

	}
	else{

		return Maybe<Transform&>();

	}

}

Maybe<const Transform&> Transform::GetParent() const{

	if (parent_ != nullptr){

		return Maybe<const Transform&>(*parent_);

	}
	else{

		return Maybe<const Transform&>();

	}

}

void Transform::Update(const Time &){

	if (parent_ != nullptr)
	{

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

void Transform::RemoveChild(Transform & child){
	
	children_.erase(std::remove(children_.begin(),
								children_.end(),
								&child),
					children_.end());

}


