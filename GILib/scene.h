#pragma once

#include <string>
#include <unordered_map>
#include <typeinfo>

#include "iterator.h"

using std::wstring;
using std::unordered_multimap;

class Component;

template <class TComponent>
class ComponentIterator;

template <class TComponent>
class ComponentConstIterator;

///Scene object such as an actor, a light, a camera and so on...
class SceneObject{

public:

	typedef unordered_multimap<size_t, Component *> ComponentMap;

	SceneObject() :
		name_(L""){}

	SceneObject(wstring name):
		name_(name){}

	///Create and add a new component to this object
	template<class TComponent, class... TArgs>
	inline ComponentIterator<TComponent> AddComponent(TArgs... args){

		Component * component = new TComponent(args...);

		///Set the owner of the component
		component->SetOwner(*this);

		return ComponentIterator<TComponent>(components_.insert(ComponentMap::value_type(typeid(TComponent).hash_code(),
																						 component)));

	}

	///Create and add a new component to this object
	template<class TComponent>
	inline ComponentIterator<TComponent> AddComponent(){

		Component * component = new TComponent();

		///Set the owner of the component
		component->SetOwner(*this);
		
		return ComponentIterator<TComponent>(components_.insert(ComponentMap::value_type(typeid(TComponent).hash_code(),
																						 component)));

	}

	///Remove the component pointed by the iterator.
	template<class TComponent>
	void RemoveComponent(ComponentIterator<TComponent> & iterator){

		auto range = components_.equal_range(typeid(TComponent).hash_code());

		auto component_ptr = &(*iterator);

		for (auto & it = range.first; it != range.second; ++it){

			if (component_ptr == (*it).second){

				components_.erase(it);

				delete component_ptr;

				return;

			}

		}
	
	}

	///Remove all the components whose type is TComponents
	template<class TComponent>
	inline void RemoveComponents(){

		components_.erase(typeid(TComponent).hash_code());

	}

	///Return the range of all components of type TComponent
	template<class TComponent>
	inline Range<TComponent, ComponentIterator<TComponent>> GetComponents(){

		auto range = components_.equal_range(typeid(TComponent).hash_code());

		return Range<TComponent, ComponentIterator<TComponent>>(ComponentIterator<TComponent>(range.first),
																ComponentIterator<TComponent>(range.second));

	}

	///Return the range of all components of type TComponent
	template<class TComponent>
	inline Range<TComponent, ComponentConstIterator<TComponent>> GetComponents() const{

		auto range = components_.equal_range(typeid(TComponent).hash_code());

		return Range<TComponent, ComponentConstIterator<TComponent>>(ComponentConstIterator<TComponent>(range.first),
																	 ComponentConstIterator<TComponent>(range.second));

	}

	///Return an iterator pointing on the first occurence of a component of type TComponent. Return a pointer to an element past the end if no component is found.
	template<class TComponent>
	inline ComponentIterator<TComponent> GetComponent(){

		return ComponentIterator<TComponent>(components_.find(typeid(TComponent).hash_code()));

	}

	///Return an iterator pointing on the first occurence of a component of type TComponent. Return a pointer to an element past the end if no component is found.
	template<class TComponent>
	inline ComponentConstIterator<TComponent> GetComponent() const{

		return ComponentConstIterator<TComponent>(components_.find(typeid(TComponent).hash_code()));

	}
	
	///Points to an element past the last component stored
	template<class TComponent>
	inline ComponentIterator<TComponent> GetEnd(){

		return ComponentIterator<TComponent>(components_.end());

	}

	///Points to an element past the last component stored
	template<class TComponent>
	inline ComponentConstIterator<TComponent> GetEnd() const{

		return ComponentConstIterator<TComponent>(components_.end());

	}
	
	///Return the name of the object
	inline const wstring & GetName() const{

		return name_;

	}

private:

	ComponentMap components_;

	wstring name_;

};

///Iterates through components
template <class TComponent>
class ComponentIterator{

public:

	ComponentIterator(SceneObject::ComponentMap::iterator iterator) :
		iterator_(iterator){};

	///Dereferencing
	TComponent & operator*(){

		return *static_cast<TComponent *>((*iterator_).second);

	}

	TComponent * operator->(){

		return static_cast<TComponent *>((*iterator_).second);

	}
	
	//Equality
	inline bool operator==(const ComponentIterator & other) const{

		return iterator_ == other.iterator_;

	}

	inline bool operator!=(const ComponentIterator & other) const{

		return iterator_ != other.iterator_;

	}

	//Forward
	inline ComponentIterator & operator++() {
		
		++iterator_;

		return *this; 
	
	}

	inline ComponentIterator operator++ (int)
	{

		ComponentIterator tmp(*this);

		++iterator_;

		return tmp;

	}

private:

	SceneObject::ComponentMap::iterator iterator_;

};

template <class TComponent>
class ComponentConstIterator{

public:

	ComponentConstIterator(SceneObject::ComponentMap::iterator iterator) :
		iterator_(iterator){};

	///Dereferencing
	const TComponent & operator*() const{

		return *static_cast<const TComponent *>((*iterator_).second);

	}

	const TComponent * operator->() const{

		return static_cast<const TComponent *>((*iterator_).second);

	}

	//Equality
	inline bool operator==(const ComponentConstIterator & other) const{

		return iterator_ == other.iterator_;

	}

	inline bool operator!=(const ComponentConstIterator & other) const{

		return iterator_ != other.iterator_;

	}

	//Forward
	inline ComponentConstIterator & operator++() {

		++iterator_;

		return *this;

	}

	inline ComponentConstIterator operator++ (int)
	{

		ComponentIterator tmp(*this);

		++iterator_;

		return tmp;

	}

private:

	SceneObject::ComponentMap::iterator iterator_;

};

///A scene object's component
class Component{

	friend class SceneObject;

public:

	virtual ~Component(){}

	///Get the owner of this component
	inline SceneObject & GetOwner(){

		return *scene_object_;

	}

	///Get the owner of this component
	inline const SceneObject & GetOwner() const{

		return *scene_object_;

	}

private:

	///Set the owner of this component
	inline void SetOwner(SceneObject & scene_object){

		scene_object_ = &scene_object;

	}

	///Owner of the component
	SceneObject * scene_object_;

};

