#include "..\include\scene.h"

#include <algorithm>
#include <assert.h>

#include "..\include\gilib.h"
#include "..\include\exceptions.h"

using namespace ::gi_lib;
using namespace ::std;

////////////////////////////////////// SCENE //////////////////////////////////////////

Scene::Scene(unique_ptr<IVolumeHierarchy> volume_hierarchy) :
main_camera_(nullptr),
volume_hierarchy_(std::move(volume_hierarchy)){}

Scene::~Scene(){

}

NodeComponent* Scene::CreateNode(const wstring& name){

	auto node = Component::Create<NodeComponent>(*this, name);

	nodes_.push_back(unique_ptr<NodeComponent>(node));

	return node;

}

TransformComponent* Scene::CreateNode(const wstring& name, const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scale){

	auto node = Component::Create<NodeComponent>(*this, name);

	auto transform = node->AddComponent<TransformComponent>(translation, rotation, scale);

	// Node and transform are the same entity. When node is deleted, transform is deleted as well.
	nodes_.push_back(unique_ptr<NodeComponent>(node));	

	return transform;

}

CameraComponent* Scene::GetMainCamera(){

	return main_camera_;

}

const CameraComponent* Scene::GetMainCamera() const{

	return main_camera_;

}

void Scene::SetMainCamera(CameraComponent* main_camera){

	main_camera_ = main_camera;

}

IVolumeHierarchy& Scene::GetVolumeHierarchy(){

	return *volume_hierarchy_;

}

const IVolumeHierarchy& Scene::GetVolumeHierarchy() const{

	return *volume_hierarchy_;

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

///////////////////////// VOLUME COMPONENT /////////////////////////

VolumeComponent::VolumeComponent() :
VolumeComponent(AABB{ Vector3f::Zero(),
					  Vector3f::Zero() }){}

VolumeComponent::VolumeComponent(const AABB& bounds) :
bounding_box_(bounds),
is_box_dirty_(true),
is_sphere_dirty_(true){}

VolumeComponent::~VolumeComponent(){}

const AABB& VolumeComponent::GetBoundingBox() const{

	if (is_box_dirty_){

		transformed_bounds_ = bounding_box_ * transform_->GetWorldTransform();

		is_box_dirty_ = false;

	}

	return transformed_bounds_;

}

const Sphere& VolumeComponent::GetBoundingSphere() const{

	if (is_sphere_dirty_){

		bounding_sphere_ = Sphere::FromAABB(GetBoundingBox());

		is_sphere_dirty_ = false;

	}

	return bounding_sphere_;

}

Observable<VolumeComponent::OnBoundsChangedEventArgs>& VolumeComponent::OnBoundsChanged(){

	return on_bounds_changed_;

}

VolumeComponent::TypeSet VolumeComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(VolumeComponent)));

	return types;

}

void VolumeComponent::Initialize(){

	// Initialize the transform component

	transform_ = GetComponent<TransformComponent>();

	on_transform_changed_lister_ = transform_->OnTransformChanged().Subscribe([this](_, _){

		SetDirty();	// The world matrix of the transform component changed.

	});

	// Plug the volume inside the volume hierarchy

	GetComponent<NodeComponent>()->GetScene()
								 .GetVolumeHierarchy()
								 .AddVolume(this);

}

void VolumeComponent::Finalize(){

	// Remove the volume from the hierarchy

	GetComponent<NodeComponent>()->GetScene()
								 .GetVolumeHierarchy()
								 .RemoveVolume(this);

}

void VolumeComponent::SetBoundingBox(const AABB& bounds){

	bounding_box_ = bounds;

	SetDirty();	// The bounds changed

}

void VolumeComponent::SetDirty(){

	is_box_dirty_ = true;
	is_sphere_dirty_ = true;

	OnBoundsChangedEventArgs args{ this };

	on_bounds_changed_.Notify(args);

}

////////////////////////////////////// MESH COMPONENT /////////////////////////////////////

MeshComponent::MeshComponent() :
mesh_(nullptr){}

MeshComponent::MeshComponent(shared_ptr<Mesh> mesh) :
VolumeComponent(mesh->GetBoundingBox()),
mesh_(mesh){}

shared_ptr<Mesh> MeshComponent::GetMesh(){

	return mesh_;

}

shared_ptr<const Mesh> MeshComponent::GetMesh() const{

	return static_pointer_cast<const Mesh>(mesh_);

}

void MeshComponent::SetMesh(shared_ptr<Mesh> mesh){

	mesh_ = mesh;

	SetBoundingBox(mesh->GetBoundingBox());

}

MeshComponent::TypeSet MeshComponent::GetTypes() const{
	
	auto types = VolumeComponent::GetTypes();

	types.insert(type_index(typeid(MeshComponent)));

	return types;

}

void MeshComponent::Initialize(){

	VolumeComponent::Initialize();

}

void MeshComponent::Finalize(){

	VolumeComponent::Finalize();

}

////////////////////////////////////// CAMERA COMPONENT /////////////////////////////////////

CameraComponent::CameraComponent(){

	projection_type_ = ProjectionType::Perspective;
	field_of_view_ = Math::DegToRad(60);
	minimum_distance_ = 1.0f;
	maximum_distance_ = 10000.f;

}

CameraComponent::~CameraComponent(){}

ProjectionType CameraComponent::GetProjectionType() const{

	return projection_type_;

}

void CameraComponent::SetProjectionType(ProjectionType projection_type){

	projection_type_ = projection_type;

}

float CameraComponent::GetFieldOfView() const{

	return field_of_view_;

}

void CameraComponent::SetFieldOfView(float field_of_view){

	field_of_view_ = field_of_view;

}

float CameraComponent::GetMinimumDistance() const{

	return minimum_distance_;

}

void CameraComponent::SetMinimumDistance(float minimum_distance){

	minimum_distance_ = minimum_distance;

}

float CameraComponent::GetMaximumDistance() const{

	return maximum_distance_;

}

void CameraComponent::SetMaximumDistance(float maximum_distance){

	maximum_distance_ = maximum_distance;

}

Frustum CameraComponent::GetViewFrustum(float aspect_ratio) const{

	if (projection_type_ == ProjectionType::Perspective){

		return Frustum(transform_->GetWorldTransform(),
					   field_of_view_,
					   minimum_distance_,
					   maximum_distance_,
					   aspect_ratio);

	}
	else{

		// Orthographic projection

		THROW(L"Orthographic projection not yet implemented!");

	}
	
}

CameraComponent::TypeSet CameraComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(CameraComponent)));

	return types;

}

void CameraComponent::Initialize(){

	// Initialize the transform component

	transform_ = GetComponent<TransformComponent>();

}

void CameraComponent::Finalize(){}