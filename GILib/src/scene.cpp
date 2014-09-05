#include "..\include\scene.h"

using namespace ::gi_lib;
using namespace ::std;

// Constructors
SceneNode::SceneNode() : SceneNode(L"", Affine3f::Identity(), {}){}

SceneNode::SceneNode(const wstring & name, const Affine3f & local_transform, initializer_list<wstring> tags):
	name_(name),
	tags_(tags.begin(), tags.end()),
	transform_(*Add<Transform>(local_transform)){}

SceneNode::SceneNode(SceneNode && other) :
name_(std::move(other.name_)),
tags_(std::move(other.tags_)),
components_(std::move(other.components_)),
transform_(*Get<Transform>()){

	// Change components' ownership
	for (auto & pair : components_){

		pair.second->owner_ = this;

	}

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

	for (auto child_index = 0; child_index <= transform_.GetChildCount(); ++child_index){

		transform_.GetChildAt(child_index).UpdateOwner(time);

	}

}