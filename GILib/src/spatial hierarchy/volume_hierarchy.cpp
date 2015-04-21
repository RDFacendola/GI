#include "..\..\include\spatial hierarchy\volume_hierarchy.h"

#include "..\..\include\scene.h"

#include "..\..\include\gilib.h"

using namespace std;
using namespace gi_lib;

///////////////////////// VOLUME HIERARCHY COMPONENT /////////////////////////

VolumeHierarchyComponent::~VolumeHierarchyComponent(){}

VolumeHierarchyComponent::TypeSet VolumeHierarchyComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(VolumeHierarchyComponent)));

	return types;

}

///////////////////////// VOLUME COMPONENT /////////////////////////

VolumeComponent::VolumeComponent() :
VolumeComponent(AABB{ Vector3f::Zero(), 
					  Vector3f::Zero() }){}

VolumeComponent::VolumeComponent(const AABB& bounds) :
bounding_box_(bounds),
hierarchy_(nullptr),
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

const Sphere& VolumeComponent::GetBoundingSphereSquared() const{

	if (is_sphere_dirty_){

		bounding_sphere_ = Sphere::FromAABBSquared(GetBoundingBox());

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

	auto scene = GetComponent<NodeComponent>();

	hierarchy_ = scene->GetComponent<VolumeHierarchyComponent>();

	hierarchy_->AddVolume(this);

	transform_ = GetComponent<TransformComponent>();

	on_transform_changed_lister_ = transform_->OnTransformChanged().Subscribe([this](_, _){

		SetDirty();	// The world matrix of the transform component changed.

	});

}

void VolumeComponent::Finalize(){

	hierarchy_->RemoveVolume(this);
	
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