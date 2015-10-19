#include "light_component.h"

using namespace gi_lib;

//////////////////////////////////// BASE LIGHT COMPONENT ////////////////////////////////////

BaseLightComponent::TypeSet BaseLightComponent::GetTypes() const {

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(BaseLightComponent)));

	return types;

}

//////////////////////////////////// POINT LIGHT COMPONENT ////////////////////////////////////

PointLightComponent::TypeSet PointLightComponent::GetTypes() const{

	auto types = BaseLightComponent::GetTypes();

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

DirectionalLightComponent::TypeSet DirectionalLightComponent::GetTypes() const{

	auto types = BaseLightComponent::GetTypes();

	types.insert(type_index(typeid(DirectionalLightComponent)));

	return types;

}

void DirectionalLightComponent::Initialize(){

	transform_component_ = GetComponent<TransformComponent>();

}

void DirectionalLightComponent::Finalize(){

	transform_component_ = nullptr;

}

//////////////////////////////////// SPOT LIGHT COMPONENT ////////////////////////////////////

SpotLightComponent::TypeSet SpotLightComponent::GetTypes() const {

	auto types = BaseLightComponent::GetTypes();

	types.insert(type_index(typeid(SpotLightComponent)));

	return types;

}

void SpotLightComponent::Initialize() {

	transform_component_ = GetComponent<TransformComponent>();

}

void SpotLightComponent::Finalize() {

	transform_component_ = nullptr;

}