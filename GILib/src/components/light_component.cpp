#include "..\..\include\components\light_component.h"

using namespace gi_lib;

//////////////////////////////////// POINT LIGHT COMPONENT ////////////////////////////////////

const Color PointLightComponent::kDefaultLightColor = Color{ { 1.0f, 1.0f, 1.0f, 1.0f } };

const float PointLightComponent::kDefaultLinearDecay = 0.0f;
const float PointLightComponent::kDefaultSquareDecay = 1.0f / (4.0f * Math::kPi);			

const float PointLightComponent::kDefaultIntensity = 1.0f;

PointLightComponent::TypeSet PointLightComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(PointLightComponent)));

	return types;

}

void PointLightComponent::Initialize(){

	transform_component_ = GetComponent<TransformComponent>();

}

void PointLightComponent::Finalize(){

	transform_component_ = nullptr;

}

//////////////////////////////////// DIRECTIONAL LIGHT COMPONENT ////////////////////////////////////

const Color DirectionalLightComponent::kDefaultLightColor = Color{ { 1.0f, 1.0f, 1.0f, 1.0f } };

const float DirectionalLightComponent::kDefaultIntensity = 1.0f;

DirectionalLightComponent::TypeSet DirectionalLightComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(DirectionalLightComponent)));

	return types;

}

void DirectionalLightComponent::Initialize(){

	transform_component_ = GetComponent<TransformComponent>();

}

void DirectionalLightComponent::Finalize(){

	transform_component_ = nullptr;

}
