#include "..\include\interface.h"

#include <algorithm>

using namespace ::gi_lib;
using namespace ::std;

namespace{

	void UnmapInterface(Interface* interface_ptr, InterfaceArbiter::InterfaceMapType& map){

		// O(#types * #interfaces_per_type)

		for (auto type : interface_ptr->GetTypes()){

			auto range = map.equal_range(type);

			while (range.first != range.second){

				if (range.first->second == interface_ptr){

					map.erase(range.first);		// An interface exists once per its types.

					break;

				}
				else{

					++(range.first);

				}

			}

		}

	}

}

/////////////////////////////// INTERFACE ARBITER /////////////////////

InterfaceArbiter::InterfaceArbiter(unique_ptr<Interface> interface_ptr){

	AddInterface(std::move(interface_ptr));

}

InterfaceArbiter::~InterfaceArbiter(){

	for (auto& interface_ptr : interfaces_){

		interface_ptr->arbiter_ = nullptr;	// The interface won't call the arbiter's destructor (since we are already deleting it).

	}

	interfaces_.clear();		// unique_ptr will take care of interface's destruction.
	interface_map_.clear();

}

Interface* InterfaceArbiter::AddInterface(unique_ptr<Interface> interface_ptr){

	auto interf = interface_ptr.get();

	interfaces_.push_back(std::move(interface_ptr));

	// Map each interface type - O(#types)

	for (auto& type : interf->GetTypes()){

		interface_map_.insert(InterfaceArbiter::InterfaceMapType::value_type(type, interf));

	}

}

void InterfaceArbiter::RemoveInterface(Interface* interface_ptr, bool suppress_destructor){

	auto it = std::find_if(interfaces_.begin(),
						   interfaces_.end(),
						   [interface_ptr](const unique_ptr<Interface>& ptr){

								return ptr.get() == interface_ptr;	

						   });

	// O(#types * #interfaces_per_type)

	if (it != interfaces_.end()){

		UnmapInterface(interface_ptr, interface_map_);

		if (suppress_destructor){

			it->release();		// This is equal to interface_ptr

		}

		interfaces_.erase(it);	// Will call the interface dtor, eventually

	}
	
}

void InterfaceArbiter::RemoveInterfaces(type_index interface_type){

	auto range = interface_map_.equal_range(interface_type);

	// Move the interfaces to be deleted at the end of the vector. - O(#interfaces * #interfaces_to_delete)

	auto end = std::remove_if(interfaces_.begin(),
							  interfaces_.end(),
							  [&range](const unique_ptr<Interface>& i){

								return any_of(range.first, 
											  range.second, 
											  [&i](const InterfaceMapType::value_type& type_interface){

												return type_interface.second == i.get();

											  });

							  });

	// Unmap the interfaces above - O(#interfaces_to_delete * #types * #interfaces_per_type)

	for (auto it = end; it != interfaces_.end(); ++it){

		UnmapInterface(it->get(), interface_map_);

	}

	// Clear the interfaces from the vector

	interfaces_.erase(end,
					  interfaces_.end());

}

Interface* InterfaceArbiter::GetInterface(type_index interface_type){

	auto it = interface_map_.find(interface_type);

	return it != interface_map_.end() ?
		   it->second :
		   nullptr;

}

InterfaceArbiter::range InterfaceArbiter::GetInterfaces(type_index interface_type){

	auto interface_range = interface_map_.equal_range(interface_type);

	return range(interface_range);

}

/////////////////////////////// INTERFACE /////////////////////////////

Interface::~Interface(){

	if (arbiter_){

		// Destructor called explicitly (ie: delete this).

		arbiter_->RemoveInterface(this, true);	// Remove this interface, no destruction is required (we're already destroying the object)

		delete arbiter_;						// Destroy the arbiter and every other interface associated to the entity.

	}
	else{

		// Destructor called via the arbiter (only this interface needs to be destroyed)

		// Do nothing

	}

}

vector<type_index> Interface::GetTypes() const{

	vector<type_index> types;

	GetTypes(types);

	return types;

}

void Interface::GetTypes(vector<type_index>& types) const{

	types.push_back(type_index(typeid(Interface)));

}

InterfaceArbiter& Interface::GetArbiter(){

	if (!arbiter_){

		// Lazy construction

		arbiter_ = new InterfaceArbiter(unique_ptr<Interface>(const_cast<Interface*>(this)));

	}

	return *arbiter_;

}

const InterfaceArbiter& Interface::GetArbiter() const{

	if (!arbiter_){

		// Lazy construction

		arbiter_ = new InterfaceArbiter(unique_ptr<Interface>(const_cast<Interface*>(this)));

	}

	return *arbiter_;

}
