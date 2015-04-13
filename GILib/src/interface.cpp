#include "..\include\interface.h"

#include <algorithm>

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

Object::~Object(){

	// Detach the interfaces from the object

	for (auto& i : interfaces_){

		i->object_ = nullptr;

	}
	
	// The unique_ptr will take care of the interfaces' destruction.

}

Interface* Object::AddInterface(unique_ptr<Interface> ptr){

	auto interf = ptr.get();

	interfaces_.push_back(std::move(ptr));

	interf->object_ = this;

	// Map each interface type - O(#types)

	for (auto& type : interf->GetTypes()){

		interface_map_.insert(InterfaceMapType::value_type(type, interf));

	}

	return interf;

}

void Object::RemoveInterface(Interface* ptr){

	auto it = std::find_if(interfaces_.begin(),
						   interfaces_.end(),
						   [ptr](const unique_ptr<Interface>& p){

								return p.get() == ptr;	

						   });

	// O(#types * #interfaces_per_type)

	if (it != interfaces_.end()){

		UnmapInterface(ptr, interface_map_);

		ptr->object_ = nullptr;
		
		interfaces_.erase(it);	// unique_ptr will call the interface destructor.
			
	}

}

Interface* Object::GetInterface(type_index interface_type){

	auto it = interface_map_.find(interface_type);

	return it != interface_map_.end() ?
		   it->second :
		   nullptr;

}

Object::InterfaceMapRange Object::GetInterfaces(type_index interface_type){

	auto r = interface_map_.equal_range(interface_type);

	return InterfaceMapRange(r);

}

/////////////////////////////// INTERFACE /////////////////////////////

Interface::Interface() :
object_(nullptr){}

Interface::~Interface(){

	if (object_ != nullptr){

		object_->RemoveInterface(this);
		
	}

}

Object* Interface::GetObject(){

	return object_;

}

const Object* Interface::GetObject() const{

	return object_;

}

vector<type_index> Interface::GetTypes() const{

	vector<type_index> types;

	GetTypes(types);

	return types;

}

void Interface::GetTypes(vector<type_index>& types) const{

	types.push_back(type_index(typeid(Interface)));

}
