#include "..\include\scene.h"

#include <algorithm>

using namespace ::gi_lib;
using namespace ::std;

/////////////////////////// SCENE NODE //////////////////////////////////////

SceneNode::SceneNode():
name_(L""),
unique_(Unique<SceneNode>::MakeUnique()),
parent_(nullptr),
position_(Translation3f(Vector3f::Identity())),
rotation_(Quaternionf::Identity()),
scale_(AlignedScaling3f(Vector3f::Ones())),
local_dirty_(true),
world_dirty_(true),
world_changed_ (true)
{

}

SceneNode::SceneNode(const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags) :
name_(name),
tags_(tags),
unique_(Unique<SceneNode>::MakeUnique()),
parent_(nullptr),
position_(position),
rotation_(rotation),
scale_(scaling),
local_dirty_(true),
world_dirty_(true),
world_changed_(true){

}

SceneNode::~SceneNode(){

	ResetParent();

	while (children_.size() > 0){

		delete children_.back();

	}

}

void SceneNode::PreUpdate(const Time & time){

	world_changed_ = false;
	
	// Update the node hierarchy
	for (auto & child : children_){

		child->PreUpdate(time);

	}

}

void SceneNode::Update(const Time & time){

	// Update the components
	for (auto & it : components_){

		if (it->IsEnabled()){

			it->Update(time);

		}

	}

	// Update the hierarchy
	for (auto & child : children_){

		child->Update(time);

	}

}

void SceneNode::PostUpdate(const Time & time){

	// Post-update the components
	for (auto & it : components_){

		if (it->IsEnabled()){

			it->PostUpdate(time);

		}

	}

	// Post-Update the hierarchy
	for (auto & child : children_){

		child->PostUpdate(time);

	}

}

// Transformation & Hierarchy

void SceneNode::SetParent(SceneNode & parent){

	ResetParent();

	// Add it to the new one
	parent.children_.push_back(this);

	parent_ = &parent;

	// The world matrix changed...
	SetDirty(true);
	
}

void SceneNode::ResetParent(){

	if (!IsRoot()){

		auto & parent_children = parent_->children_;

		// Erase this node from the current parent
		parent_children.erase(std::remove(parent_children.begin(),
			parent_children.end(),
			this),
			parent_children.end());

		parent_ = nullptr;

	}
	
}

vector<reference_wrapper<SceneNode>> SceneNode::FindNodeByName(const wstring & name){

	vector<reference_wrapper<SceneNode>> result;

	FindNodeByName(result, name);

	return result;

}

vector<reference_wrapper<SceneNode>> SceneNode::FindNodeByTag(std::initializer_list<wstring> tags){

	vector<reference_wrapper<SceneNode>> result;

	FindNodeByTag(result, tags);

	return result;

}

void SceneNode::SetDirty(bool world_only){

	local_dirty_ |= !world_only;
	world_dirty_ = true;

	world_changed_ = true;

	// Dirtens every world matrix on the children

	for (auto child : children_){

		child->SetDirty(true);

	}

}

void SceneNode::UpdateLocalTransform(){

	if (local_dirty_){

		local_transform_ = scale_ * rotation_ * position_;

		local_dirty_ = false;
		
		// this node and its children have their world transform dirty...
		SetDirty(true);

	}

}

void SceneNode::UpdateWorldTransform(){

	UpdateLocalTransform();

	if (world_dirty_){

		if (!IsRoot()){

			// Local transform first, world transform then
			world_transform_ = parent_->GetWorldTransform() * local_transform_;

		}
		else{

			// A root have no parent
			world_transform_ = local_transform_;

		}
		
		world_dirty_ = false;

	}

}

void SceneNode::FindNodeByName(vector<reference_wrapper<SceneNode>> & nodes, const wstring & name){

	// Check this node
	if (this->name_ == name){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (auto & child : children_){

		child->FindNodeByName(nodes, name);

	}

}

void SceneNode::FindNodeByTag(vector<reference_wrapper<SceneNode>> & nodes, std::initializer_list<wstring> tags){

	// Check this node
	if (HasTags(tags)){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (auto & child : children_){

		child->FindNodeByTag(nodes, tags);

	}

}

/////////////////////////// SCENE ///////////////////////////////////////////

Scene::Scene() :
root_(){}

void Scene::Update(const Time & time){

	// Pre update
	root_.PreUpdate(time);

	// Update the hierarchy starting from the root.
	root_.Update(time);

	// Post update.
	root_.PostUpdate(time);

}