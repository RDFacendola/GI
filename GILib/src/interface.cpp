#include "..\include\interface.h"

#include <algorithm>

#include "..\include\exceptions.h"

using namespace ::gi_lib;
using namespace ::std;

namespace{

	void UnmapInterface(Interface* interface_ptr, Object::InterfaceMapType& map){

		// O(#types * #interfaces_per_type)

		for (auto type : interface_ptr->GetTypes()){

			auto range = map.equal_range(type);

			while (range.first != range.second){

				if (range.first->second == interface_ptr){

					map.erase(range.first);		// An interface exists at most once for each type.

					break;

				}
				else{

					++(range.first);

				}

			}

		}

	}

}

/////////////////////////////// OBJECT ////////////////////////////////////

Object::Object() :
alive_(true){}

Object::~Object(){

	alive_ = false;

	for (auto p : interface_set_){

		delete p;

	}

}

void Object::AddInterface(Interface* ptr){

	if (alive_){

		interface_set_.insert(ptr);

		// Map each interface type - O(#types)

		for (auto& type : ptr->GetTypes()){

			interface_map_.insert(InterfaceMapType::value_type(type, ptr));

		}

	}
	else{

		// This happens only when an interface is added during another interface's destructor.
		// This is likely to be a poor design choice (an explicit method would be better).

		THROW(L"Unable to add an interface to an object that's being destroyed");

	}
	
}

void Object::RemoveInterface(Interface* ptr){

	// If the object is dying, we don't bother deleting a single interface...

	if (alive_){

		auto it = interface_set_.find(ptr);

		// O(#types * #interfaces_per_type)

		if (it != interface_set_.end()){

			UnmapInterface(ptr, interface_map_);

			delete *it;					// Delete the interface

			interface_set_.erase(it);	// Remove the element
			
		}

	}
	
}

Interface* Object::GetInterface(type_index interface_type){

	auto it = interface_map_.find(interface_type);

	return it != interface_map_.end() ?
		   it->second :
		   nullptr;

}

Range < Object::InterfaceMapType::iterator > Object::GetInterfaces(type_index interface_type){

	return Range < InterfaceMapType::iterator >(interface_map_.equal_range(interface_type));

}

void Object::Destroy(){

	delete this;

}

/////////////////////////////// INTERFACE /////////////////////////////

Interface::Interface(Object& object) :
object_(object){

	object_.AddInterface(this);

}

Interface::~Interface(){

	object_.RemoveInterface(this);

}

Object& Interface::GetComposite(){

	return object_;

}

const Object& Interface::GetComposite() const{

	return object_;

}

set<type_index> Interface::GetTypes() const{

	// The set is needed to prevent multiple inclusion of the same type
	
	set<type_index> types;
		
	GetTypes(types);

	return types;

}

void Interface::GetTypes(set<type_index>& types) const{

	types.insert(type_index(typeid(Interface)));

}


