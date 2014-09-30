#include "..\include\scene.h"

#include <algorithm>

using namespace ::gi_lib;
using namespace ::std;

/////////////////////////// SCENE ///////////////////////////////////////////

Scene::Scene():
	root_( std::make_unique<SceneNode>(*this) ){}

Scene::~Scene(){

	nodes_.clear();

}

void Scene::DestroyNode(SceneNode & node){

	auto key = node.GetUniqueID();

	nodes_.erase(key);

}

void Scene::Update(const Time & time){

	//Update traverse the node hierarchy Transform-wise starting from the root.
	root_->Update(time);

}

void Scene::SetRoot(SceneNode & node){

	node.SetParent(*root_);

}

/////////////////////////// SCENE NODE //////////////////////////////////////

SceneNode::SceneNode(Scene & scene) : SceneNode(scene, L"", Affine3f::Identity(), {}){}

SceneNode::SceneNode(Scene & scene, const wstring & name, const Affine3f & local_transform, initializer_list<wstring> tags):
	scene_(scene),
	name_(name),
	tags_(tags.begin(), tags.end()),
	transform_(*Add<Transform>(local_transform)),
	unique_(Unique<SceneNode>::MakeUnique()){

}

SceneNode::SceneNode(SceneNode && other) :
	scene_(other.scene_),
	name_(std::move(other.name_)),
	tags_(std::move(other.tags_)),
	components_(std::move(other.components_)),
	transform_(*Get<Transform>()),
	unique_(other.unique_){
	
	other.unique_ = Unique<SceneNode>::kNull;

	// Change components' ownership
	for (auto & pair : components_){

		pair.second->owner_ = this;

	}

	scene_.SetRoot(*this);

}

bool SceneNode::HasTags(std::initializer_list<wstring> tags){

	bool result = true;

	for (auto & tag : tags){

		result &= HasTag(tag);

	}

	return result;

}

std::vector<reference_wrapper<SceneNode>> SceneNode::FindNodeByName(const wstring & name){

	vector<reference_wrapper<SceneNode>> result;

	FindNodeByName(result, name);

	return result;

}

std::vector<reference_wrapper<SceneNode>> SceneNode::FindNodeByTag(std::initializer_list<wstring> tags){
	
	vector<reference_wrapper<SceneNode>> result;

	FindNodeByTag(result, tags);

	return result;

}

void SceneNode::Update(const Time & time){

	// Update the components
	for (auto & it : components_){

		if (it.second->IsEnabled()){

			it.second->Update(time);

		}

	}

	// Update the hierarchy
	UpdateHierarchy(time);

}

void SceneNode::UpdateHierarchy(const Time & time){

	// Access child scene nodes by transform components.

	for (auto child_index = 0; child_index < transform_.GetChildCount(); ++child_index){

		transform_.GetChildAt(child_index).UpdateOwner(time);

	}

}

void SceneNode::FindNodeByName(std::vector<reference_wrapper<SceneNode>> & nodes, const wstring & name){

	// Check this node
	if (this->name_ == name){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (int child_index = 0; child_index < transform_.GetChildCount(); ++child_index){

		transform_.GetChildAt(child_index)
				  .GetOwner()
				  .FindNodeByName(nodes, name);

	}

}

void SceneNode::FindNodeByTag(std::vector<reference_wrapper<SceneNode>> & nodes, std::initializer_list<wstring> tags){

	// Check this node
	if (HasTags(tags)){

		nodes.push_back(*this);

	}

	// Depth-first (keeps stack size limited)
	for (int child_index = 0; child_index < transform_.GetChildCount(); ++child_index){

		transform_.GetChildAt(child_index)
			.GetOwner()
			.FindNodeByTag(nodes, tags);

	}
}