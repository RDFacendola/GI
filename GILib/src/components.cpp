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

	if (parent_ != nullptr){

		parent_->RemoveChild(*this);

		for (auto & child : children_){

			child->parent_ = nullptr;
			child->Attach(*parent_);

		}

	}
	else{

		for (auto & child : children_){

			child->parent_ = nullptr;

		}

	}

	children_.clear();

}

void Transform::Attach(Transform & parent){

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

void Transform::AddChild(Transform & child){

	children_.push_back(&child);

}

void Transform::RemoveChild(Transform & child){
	
	children_.erase(std::remove(children_.begin(),
								children_.end(),
								&child),
					children_.end());

}


