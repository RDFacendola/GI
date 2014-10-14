#include "..\include\scene.h"

#include <algorithm>

using namespace ::gi_lib;
using namespace ::std;

/////////////////////////// SCENE ///////////////////////////////////////////

Scene::Scene():
	root_( *this ){}

Scene::~Scene(){

	nodes_.clear();

}

void Scene::DestroyNode(SceneNode & node){

	auto key = node.GetUniqueID();

	nodes_.erase(key);

}

void Scene::Update(const Time & time){

	//Update traverse the node hierarchy Transform-wise starting from the root.
	root_.Update(time);

}

/////////////////////////// SCENE NODE //////////////////////////////////////

SceneNode::SceneNode(Scene & scene) : SceneNode(scene.GetRoot(), L"", Translation3f(Vector3f::Zero()), Quaternionf::Identity(), AlignedScaling3f(Vector3f::Ones()), {}){}

SceneNode::SceneNode(SceneNode & parent, const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags) :
scene_(parent.GetScene()),
name_(name),
tags_(tags),
unique_(Unique<SceneNode>::MakeUnique()),
parent_(&parent),
position_(position),
rotation_(rotation),
scale_(scaling),
local_dirty_(true),
world_dirty_(true){

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

		child.get().Update(time);

	}

}

// Transformation & Hierarchy

void SceneNode::SetParent(SceneNode & parent){

	auto & parent_children = parent_->children_;

	// Erase this node from the current parent
	parent_children.erase(std::remove_if(parent_children.begin(),
		parent_children.end(),
		[this](const std::reference_wrapper<SceneNode> & reference){ return reference.get() == *this; }),
		parent_children.end());

	// Add it to the new one
	parent.children_.push_back(*this);

	parent_ = &parent;

	// The world matrix changed...
	SetDirty(true);


}

SceneNode::ChildrenList SceneNode::FindNodeByName(const wstring & name){

	SceneNode::ChildrenList result;

	FindNodeByName(result, name);

	return result;

}

SceneNode::ChildrenList SceneNode::FindNodeByTag(std::initializer_list<wstring> tags){

	SceneNode::ChildrenList result;

	FindNodeByTag(result, tags);

	return result;

}

void SceneNode::SetDirty(bool world_only){

	local_dirty_ = !world_only;
	world_dirty_ = true;

	// Dirtens every world matrix on the children

	for (auto & child : children_){

		child.get().SetDirty(true);

	}

}

void SceneNode::UpdateLocalTransform(){

	if (local_dirty_){

		local_transform_ = position_ * (rotation_ * scale_);

		local_dirty_ = false;

	}

}

void SceneNode::UpdateWorldTransform(){

	if (world_dirty_){

		// Local transform first, world transform then
		world_transform_ = parent_->GetWorldTransform() * local_transform_;

		world_dirty_ = false;

	}

}

void SceneNode::FindNodeByName(std::vector<reference_wrapper<SceneNode>> & nodes, const wstring & name){

	// Check this node
	if (this->name_ == name){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (auto & child : children_){

		child.get().FindNodeByName(nodes, name);

	}

}

void SceneNode::FindNodeByTag(std::vector<reference_wrapper<SceneNode>> & nodes, std::initializer_list<wstring> tags){

	// Check this node
	if (HasTags(tags)){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (auto & child : children_){

		child.get().FindNodeByTag(nodes, tags);

	}

}