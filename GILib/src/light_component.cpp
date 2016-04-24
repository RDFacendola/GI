#include "light_component.h"

#include <limits>

using namespace gi_lib;

//////////////////////////////////// BASE LIGHT COMPONENT ////////////////////////////////////

BaseLightComponent::TypeSet BaseLightComponent::GetTypes() const {

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(BaseLightComponent)));

	return types;

}

void BaseLightComponent::ComputeBounds(bool notify) {

	ComputeBounds();

	if (notify) {

		VolumeComponent::NotifyChange();

	}

}

void BaseLightComponent::Initialize() {

	transform_ = GetComponent<TransformComponent>();
	
	ComputeBounds(false);
	
	on_transform_changed_lister_ = transform_->OnTransformChanged().Subscribe([this](_, _) {

		ComputeBounds();	// The world matrix of the transform component changed.

	});

	GetComponent<NodeComponent>()->GetScene()
								 .GetLightHierarchy()
								 .AddVolume(this);
}

void BaseLightComponent::Finalize() {

	GetComponent<NodeComponent>()->GetScene()
								 .GetLightHierarchy()
								 .RemoveVolume(this);
	
	transform_ = nullptr;
	
}

//////////////////////////////////// POINT LIGHT COMPONENT ////////////////////////////////////

const float PointLightComponent::kDefaultCutoff = 0.001f;

PointLightComponent::TypeSet PointLightComponent::GetTypes() const{

	auto types = BaseLightComponent::GetTypes();

	types.insert(type_index(typeid(PointLightComponent)));

	return types;

}

PointLightComponent::PointLightComponent(const Color& color, float radius) :
	BaseLightComponent(color),
	constant_factor_(1.0f),
	linear_factor_(2.0f / radius),
	quadratic_factor_(1.0f / (radius * radius)),
	cutoff_(kDefaultCutoff){

}

void PointLightComponent::SetRadius(float radius) {

	constant_factor_ = 1.0f;
	linear_factor_ = 2.0f / radius;
	quadratic_factor_ = 1.0f / (radius * radius);

	BaseLightComponent::ComputeBounds(true);

}

void PointLightComponent::ComputeBounds() {

	// The point light influence is theoretically infinite, however we can cut its influence when its contribution drops below a given threshold

	// cutoff = 1 / (kq*d^2 + kl*d + kc)  - Solve for "d"

	const float r_cutoff = 1.0f / cutoff_;

	float influence_radius;

	if (quadratic_factor_ > 0.0f) {

		float delta = linear_factor_ * linear_factor_ - 4.0f * quadratic_factor_ * (constant_factor_ - r_cutoff);	// b^2 - 4ac 

		influence_radius = (-linear_factor_ + std::sqrtf(delta)) / (2.0f * quadratic_factor_);					// (-b + sqrt(delta)) / 2a, ignoring -sqrt(delta) as it would make the influence radius negative.

	}
	else {

		influence_radius = (r_cutoff - constant_factor_) / linear_factor_;

	}

	assert(influence_radius > 0.0f);		// oh snap!

	bounds_ = Sphere{ GetTransformComponent().GetPosition(),
					  influence_radius };

}

//////////////////////////////////// DIRECTIONAL LIGHT COMPONENT ////////////////////////////////////

DirectionalLightComponent::TypeSet DirectionalLightComponent::GetTypes() const{

	auto types = BaseLightComponent::GetTypes();

	types.insert(type_index(typeid(DirectionalLightComponent)));

	return types;

}

//////////////////////////////////// SPOT LIGHT COMPONENT ////////////////////////////////////

SpotLightComponent::TypeSet SpotLightComponent::GetTypes() const {

	auto types = BaseLightComponent::GetTypes();

	types.insert(type_index(typeid(SpotLightComponent)));

	return types;

}

IntersectionType SpotLightComponent::TestAgainst(const Frustum&) const {

	THROW(L"NOT YET IMPLEMENTED");
	/*return IntersectionType::kIntersect;*/

}

IntersectionType SpotLightComponent::TestAgainst(const AABB&) const {

	THROW(L"NOT YET IMPLEMENTED");
	/*return IntersectionType::kIntersect;*/

}

IntersectionType SpotLightComponent::TestAgainst(const Sphere&) const {

	THROW(L"NOT YET IMPLEMENTED");
	/*return IntersectionType::kIntersect;*/

}

void SpotLightComponent::ComputeBounds() {

	THROW(L"NOT YET IMPLEMENTED");

}