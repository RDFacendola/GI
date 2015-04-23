#include "..\..\include\spatial hierarchy\volume_hierarchy.h"

using namespace std;
using namespace gi_lib;

///////////////////////// VOLUME HIERARCHY COMPONENT /////////////////////////

VolumeHierarchyComponent::~VolumeHierarchyComponent(){}

VolumeHierarchyComponent::TypeSet VolumeHierarchyComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(VolumeHierarchyComponent)));

	return types;

}