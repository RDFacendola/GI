#include "..\include\scene.h"

#include <algorithm>
#include <assert.h>

using namespace ::gi_lib;
using namespace ::std;

////////////////////////////////////// SCENE //////////////////////////////////////////

Scene::Scene(){

}

////////////////////////////////////// NODE COMPONENT /////////////////////////////////////

NodeComponent::NodeComponent(Scene& scene, const wstring& name) :
scene_(scene),
name_(name),
uid_(Unique<NodeComponent>::MakeUnique()){}

NodeComponent::~NodeComponent(){}

Scene& NodeComponent::GetScene(){

	return scene_;

}

const Scene& NodeComponent::GetScene() const{

	return scene_;

}

const wstring& NodeComponent::GetName() const{


	return name_;

}

const Unique<NodeComponent> NodeComponent::GetUid() const{

	return uid_;

}

NodeComponent::TypeSet NodeComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(NodeComponent)));

	return types;

}

void NodeComponent::Initialize(){}

void NodeComponent::Finalize(){}

////////////////////////////////////// TRANSFORM COMPONENT /////////////////////////////////////

TransformComponent::TransformComponent() :
TransformComponent(Translation3f(Vector3f::Zero()),
				   Quaternionf::Identity(),
				   AlignedScaling3f(Vector3f::Ones())){}

TransformComponent::TransformComponent(const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scale) :
parent_(nullptr),
translation_(translation),
rotation_(rotation),
scale_(scale),
local_dirty_(true),
world_dirty_(true){}

const Translation3f & TransformComponent::GetTranslation() const{

	return translation_;

}

void TransformComponent::SetTranslation(const Translation3f & translation){

	translation_ = translation;

	SetDirty(false);	// World and local

}

const Quaternionf & TransformComponent::GetRotation() const{

	return rotation_;

}

void TransformComponent::SetRotation(const Quaternionf & rotation){

	rotation_ = rotation;

	SetDirty(false);	// World and local

}

const AlignedScaling3f & TransformComponent::GetScale() const{

	return scale_;

}

void TransformComponent::SetScale(const AlignedScaling3f & scale){

	scale_ = scale;

	SetDirty(false);	// World and local

}

const Affine3f & TransformComponent::GetLocalTransform() const{

	if (local_dirty_){

		local_transform_ = scale_ * rotation_ * translation_;

		local_dirty_ = false;

	}

	return local_transform_;

}

const Affine3f & TransformComponent::GetWorldTransform() const{

	if (world_dirty_){

		auto local_transform = GetLocalTransform();

		world_transform_ = parent_ ?
						   parent_->GetWorldTransform() * local_transform :
						   local_transform;

		world_dirty_ = false;

	}

	return world_transform_;

}

TransformComponent* TransformComponent::GetParent(){

	return parent_;

}

const TransformComponent* TransformComponent::GetParent() const{

	return parent_;

}

void TransformComponent::SetParent(TransformComponent* parent){

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

TransformComponent::range TransformComponent::GetChildren(){

	return range(children_.begin(),
				 children_.end());

}

TransformComponent::const_range TransformComponent::GetChildren() const{

	return const_range(children_.cbegin(),
					   children_.cend());

}

TransformComponent::TypeSet TransformComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(TransformComponent)));

	return types;

}

void TransformComponent::Initialize(){}

void TransformComponent::Finalize(){}

void TransformComponent::SetDirty(bool world_only){

	local_dirty_ |= !world_only;

	world_dirty_ = true;

	OnTransformChangedEventArgs args{ this };

	on_transform_changed_.Notify(args);

	// Invalidate the children recursively

	for (auto& child : children_){

		child->SetDirty(true);	// Children's matrix needs to be recalculated

	}

}

Observable<TransformComponent::OnTransformChangedEventArgs>& TransformComponent::OnTransformChanged(){

	return on_transform_changed_;

}

////////////////////////////////////// STATIC MESH COMPONENT /////////////////////////////////////

StaticMeshComponent::StaticMeshComponent() :
StaticMeshComponent(nullptr){}

StaticMeshComponent::StaticMeshComponent(shared_ptr<Mesh> mesh) :
VolumeComponent(mesh->GetBounds()),
mesh_(mesh){}

shared_ptr<Mesh> StaticMeshComponent::GetMesh(){

	return mesh_;

}

shared_ptr<const Mesh> StaticMeshComponent::GetMesh() const{

	return static_pointer_cast<const Mesh>(mesh_);

}

void StaticMeshComponent::SetMesh(shared_ptr<Mesh> mesh){

	mesh_ = mesh;

	SetBoundingBox(mesh->GetBounds());

}

StaticMeshComponent::TypeSet StaticMeshComponent::GetTypes() const{
	
	auto types = VolumeComponent::GetTypes();

	types.insert(type_index(typeid(StaticMeshComponent)));

	return types;

}

void StaticMeshComponent::Initialize(){

	VolumeComponent::Initialize();

}

void StaticMeshComponent::Finalize(){

	VolumeComponent::Finalize();

}

////////////////////////////////////// CAMERA COMPONENT /////////////////////////////////////
