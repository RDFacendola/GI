#include "..\include\scene.h"

#include <algorithm>
#include <assert.h>

using namespace ::gi_lib;
using namespace ::std;

////////////////////////////////////// SCENE //////////////////////////////////////////

Scene::Scene(){

}

////////////////////////////////////// SCENE NODE /////////////////////////////////////

SceneNode::SceneNode(Scene& scene, const wstring& name) :
scene_(scene),
name_(name),
uid_(Unique<SceneNode>::MakeUnique()){}

SceneNode::~SceneNode(){}

Scene& SceneNode::GetScene(){

	return scene_;

}

const Scene& SceneNode::GetScene() const{

	return scene_;

}

const wstring& SceneNode::GetName() const{


	return name_;

}

const Unique<SceneNode> SceneNode::GetUid() const{

	return uid_;

}

bool SceneNode::operator==(const SceneNode & other) const{

	return uid_ == other.uid_;

}

bool SceneNode::operator!=(const SceneNode & other) const{

	return uid_ != other.uid_;

}

SceneNode::TypeSet SceneNode::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(SceneNode)));

	return types;

}

void SceneNode::Initialize(){}

void SceneNode::Finalize(){}

////////////////////////////////////// TRANSFORM /////////////////////////////////////

Transform::Transform() :
Transform(Translation3f(Vector3f::Zero()), 
		  Quaternionf::Identity(),
		  AlignedScaling3f(Vector3f::Ones())){}

Transform::Transform(const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scaling) :
parent_(nullptr),
translation_(translation),
rotation_(rotation),
scale_(scaling),
local_dirty_(true),
world_dirty_(true){}

const Translation3f & Transform::GetTranslation() const{

	return translation_;

}

void Transform::SetTranslation(const Translation3f & translation){

	translation_ = translation;

	SetDirty(false);	// World and local

}

const Quaternionf & Transform::GetRotation() const{

	return rotation_;

}

void Transform::SetRotation(const Quaternionf & rotation){

	rotation_ = rotation;

	SetDirty(false);	// World and local

}

const AlignedScaling3f & Transform::GetScale() const{

	return scale_;

}

void Transform::SetScale(const AlignedScaling3f & scale){

	scale_ = scale;

	SetDirty(false);	// World and local

}

const Affine3f & Transform::GetLocalTransform() const{

	if (local_dirty_){

		local_transform_ = scale_ * rotation_ * translation_;

		local_dirty_ = false;

		SetDirty(true);	 // The world matrix needs to be recalculated

	}

	return local_transform_;

}

const Affine3f & Transform::GetWorldTransform() const{

	auto local_transform = GetLocalTransform();

	if (world_dirty_){

		world_transform_ = parent_ ?
						   parent_->GetWorldTransform() * local_transform :
						   local_transform;

		world_dirty_ = false;

	}

	return world_transform_;

}

Transform* Transform::GetParent(){

	return parent_;

}

const Transform* Transform::GetParent() const{

	return parent_;

}

void Transform::SetParent(Transform* parent){

	// Remove from the old parent
	if (parent_ != nullptr){

		auto& parent_children = parent->children_;

		parent_children.erase(std::remove(parent_children.begin(),
										  parent_children.end(),
										  this),
							  parent_children.end());

	}

	// Add to the new one
	parent_ = parent;

	if (parent != nullptr){

		parent->children_.push_back(this);

	}

}

Transform::range Transform::GetChildren(){

	return range(children_.begin(),
				 children_.end());

}

Transform::const_range Transform::GetChildren() const{

	return const_range(children_.cbegin(),
					   children_.cend());

}

Transform::TypeSet Transform::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(Transform)));

	return types;

}

void Transform::Initialize(){}

void Transform::Finalize(){}

void Transform::SetDirty(bool world_only) const{

	local_dirty_ |= !world_only;
	world_dirty_ = true;

	// Dirtens every world matrix on the children

	for (auto & child : children_){

		child->SetDirty(true);

	}

}
