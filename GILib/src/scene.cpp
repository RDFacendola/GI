#include "..\include\scene.h"

#include <algorithm>
#include <assert.h>

using namespace ::gi_lib;
using namespace ::std;

/////////////////////////// __SCENE NODE ////////////////////////////////////

void __SceneNode::RemoveComponent(__SceneComponent& component){

	// O(#types * #avg_components_per_type)

	for (auto type : component.GetTypes()){

		auto range = component_map_.equal_range(type);

		for (auto it = range.first; it != range.second; ++it){

			if (it->second.get() == std::addressof(component)){

				component_map_.erase(it);

				break;	// One component per type maximum.

			}

		}

	}

}

__SceneNode::IteratorPair __SceneNode::AddComponent(shared_ptr<__SceneComponent> component){

	// Adds a new key for each type the component can be casted to.

	for (auto type : component->GetTypes()){

		component_map_.insert(ComponentMapType::value_type{ type, component });

	}

}

void __SceneNode::RemoveComponents(type_index component_type){
	
	__SceneComponent* component;

	auto it = component_map_.find(component_type);

	while (it != component_map_.end()){

		component = it->second.get();

		++it;

		RemoveComponent(*component);

	}

}

/////////////////////////// SCENE NODE //////////////////////////////////////

SceneNode::SceneNode(Scene & scene, const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags) :
scene_(scene),
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

	// Will destroy the children recursively...

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
	
	// Prevents the root from changing parent...
	assert(parent_ != nullptr);

	//Move from the old parent to the new one

	parent.AddNode(parent_->MoveNode(*this));

	parent_ = addressof(parent);

	// The world matrix changed...
	SetDirty(true);
	
}

SceneNode & SceneNode::AddNode(unique_ptr<SceneNode> && node){
	
	auto & node_ref = *node;

	children_.push_back(std::move(node));

	node_ref.parent_ = this;

	return node_ref;
	
}

unique_ptr<SceneNode> SceneNode::MoveNode(SceneNode & node){
	
	auto it = std::find_if(children_.begin(),
		children_.end(),
		[&node](unique_ptr<SceneNode> & node_ptr){

		return node_ptr.get() == addressof(node);

	});

	if (it != children_.end()){

		// Move the found node outside the children array and erase the old position.

		auto node_ptr = std::move(*it);

		children_.erase(it);

		return node_ptr;

	}
	else{

		return nullptr;	//Should never happen, though.

	}
	
}

void SceneNode::DestroyNode(SceneNode & node){

	children_.erase(std::remove_if(children_.begin(),
		children_.end(),
		[&node](const unique_ptr<SceneNode> & node_ptr){

			return node_ptr.get() == addressof(node);

		}),
		children_.end());

}

void SceneNode::SetDirty(bool world_only) const{

	local_dirty_ |= !world_only;
	world_dirty_ = true;

	world_changed_ = true;

	// Dirtens every world matrix on the children

	for (auto & child : children_){

		child->SetDirty(true);

	}

}

void SceneNode::UpdateLocalTransform() const{

	if (local_dirty_){

		local_transform_ = scale_ * rotation_ * position_;

		local_dirty_ = false;
		
		// this node and its children have their world transform dirty...
		SetDirty(true);

	}

}

void SceneNode::UpdateWorldTransform() const{

	UpdateLocalTransform();

	if (world_dirty_){

		if (GetParent()){

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

void SceneNode::FindNodeByName(const wstring & name, vector<SceneNode *> & nodes){

	if (name_ == name){

		nodes.push_back(this);

	}

	for (auto & child : children_){

		child->FindNodeByName(name, nodes);

	}

}

void SceneNode::FindNodeByTag(std::initializer_list<wstring> & tags, vector<SceneNode *> nodes){

	if (HasTags(tags)){

		nodes.push_back(this);

	}

	for (auto & child : children_){

		child->FindNodeByTag(tags, nodes);

	}

}

/////////////////////////// SCENE ///////////////////////////////////////////

Scene::Scene(){

	root_ = make_unique<SceneNode>(*this, L"", Translation3f(Vector3f::Zero()), Quaternionf::Identity(), AlignedScaling3f(Vector3f::Ones()), initializer_list < wstring > {});
	bvh_ = make_unique<Octree>();

}

Scene::~Scene(){

	root_ = nullptr;
	bvh_ = nullptr;

}

SceneNode & Scene::CreateNode(const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags){

	return root_->AddNode(make_unique<SceneNode>(*this, name, position, rotation, scaling, tags));

}

SceneNode & Scene::CreateNode(){

	return CreateNode(L"", Translation3f(Vector3f::Zero()), Quaternionf::Identity(), AlignedScaling3f(Vector3f::Ones()), {});

}

void Scene::DestroyNode(SceneNode & node){
	
	auto parent = node.GetParent();

	if (parent){

		parent->DestroyNode(node);

	}
	else{

		// We are deleting the root and replacing it with a new one!
		root_ = make_unique<SceneNode>(*this, L"", Translation3f(Vector3f::Zero()), Quaternionf::Identity(), AlignedScaling3f(Vector3f::Ones()), initializer_list < wstring > {});

	}

}

vector<SceneNode *> Scene::FindNodeByName(const wstring & name){

	vector<SceneNode *> nodes;

	root_->FindNodeByName(name, nodes);

	return nodes;

}

vector<SceneNode *> Scene::FindNodeByTag(std::initializer_list<wstring> tags){

	vector<SceneNode *> nodes;

	root_->FindNodeByTag(tags, nodes);

	return nodes;

}

void Scene::Update(const Time & time){

	root_->PreUpdate(time);
	root_->Update(time);
	root_->PostUpdate(time);

}

void Scene::AddCamera(Camera & camera){

	cameras_.push_back(std::addressof(camera));

	SortCamerasByPriority();
	
}

void Scene::RemoveCamera(Camera & camera){

	cameras_.erase(std::remove(cameras_.begin(),
		cameras_.end(),
		std::addressof(camera)),
		cameras_.end());

	SortCamerasByPriority();

}

void Scene::SortCamerasByPriority(){

	std::sort(cameras_.begin(),
		cameras_.end(),
		[](const Camera * first, const Camera * second){

		return first->GetPriority() < second->GetPriority();

	});

}