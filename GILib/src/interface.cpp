#include "..\include\interface.h"

#include <algorithm>

#include "..\include\exceptions.h"

using namespace ::gi_lib;
using namespace ::std;

namespace{

	void UnmapInterface(Component* component, Component::ComponentMap& map){

		// O(#types * #interfaces_per_type)

		for (auto type : component->GetTypes()){

			auto range = map.equal_range(type);

			while (range.first != range.second){

				if (range.first->second == component){

					map.erase(range.first);		

					break; // At most once per type

				}
				else{

					++(range.first);

				}

			}

		}

	}

}

/////////////////////////////// COMPONENT :: ARBITER //////////////////////////////

class Component::Arbiter{

public:

	using ComponentSet = set < Component* >;

	using ComponentMap = unordered_multimap < type_index, Component* >;

	using range = Component::map_range;

	Arbiter();

	~Arbiter();

	void AddComponent(Component* component);

	void RemoveComponent(Component* component);

	void RemoveAll();

	range GetComponents(type_index type);

private:

	void DeleteComponent(ComponentSet::iterator it);

	ComponentSet component_set_;

	ComponentMap component_map_;

	bool autodestroy_;

};

Component::Arbiter::Arbiter() :
autodestroy_(true){}

Component::Arbiter::~Arbiter(){

	autodestroy_ = false;		// This ensures that this destructor is called exactly once

	// Loop needed because Component::Finalize may still add or remove other components. An iteration may not be sufficient.

	while (!component_set_.empty()){

		DeleteComponent(component_set_.begin());

	}

}

void Component::Arbiter::AddComponent(Component* component){

	component_set_.insert(component);

	// Map each component type - O(#types)

	for (auto& type : component->GetTypes()){

		component_map_.insert(ComponentMap::value_type(type, component));

	}

	component->arbiter_ = this;

	// The initialization must occur after the registration because if Component::Initialize removes the last interface, 
	// the arbiter would be destroyed erroneously.

	component->Initialize();		// Cross-component construction

}

void Component::Arbiter::RemoveComponent(Component* component){

	auto it = component_set_.find(component);
	
	if (it != component_set_.end()){

		DeleteComponent(it);

		if (autodestroy_ &&
			component_set_.empty()){

			delete this;	// Autodestruction

		}

	}
	
}

Component::Arbiter::range Component::Arbiter::GetComponents(type_index type){

	return range(component_map_.equal_range(type));

}

void Component::Arbiter::DeleteComponent(ComponentSet::iterator it){

	auto component = *it;

	component->Finalize();							// Cross-component destruction

	component->arbiter_ = nullptr;					// No really needed, ensures that the dtor of the component won't access the other components.

	component_set_.erase(it);

	UnmapInterface(component, component_map_);

	delete component;								// Independent destruction

}


/////////////////////////////// COMPONENT /////////////////////////////////////////

Component::Component() :
arbiter_(nullptr){}

Component::~Component(){}

void Component::RemoveComponent(){

	arbiter_->RemoveComponent(this);

}

void Component::Dispose(){

	delete arbiter_;

}

Component::TypeSet Component::GetTypes() const{

	set<type_index> types;

	types.insert(type_index(typeid(Component)));

	return types;

}

Component::map_range Component::GetComponents(type_index type) const{

	return arbiter_->GetComponents(type);

}

void Component::Setup(Arbiter* arbiter){

	arbiter->AddComponent(this);

}

void Component::Setup(){

	Setup(new Arbiter());

}