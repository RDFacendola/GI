/// \file interface.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <set>
#include <iterator>

#include "macros.h"
#include "range.h"

using std::type_index;
using std::unordered_multimap;
using std::set;

namespace gi_lib{

	/// \brief Base component.
	/// \auhtor Raffaele D. Facendola.
	class Component{

	public:

		using ComponentSet = set < Component* >;

		using ComponentMap = unordered_multimap < type_index, Component* >;

		using TypeSet = set < type_index >;

		template <typename TComponent>
		struct ReferenceMap{

			TComponent& operator()(ComponentMap::iterator& iterator);

		};

		template <typename TComponent>
		struct PointerMap{

			TComponent* operator()(ComponentMap::iterator& iterator);

		};

		template <typename TComponent>
		using iterator = IteratorWrapper < ComponentMap::iterator, TComponent, ReferenceMap<TComponent>, PointerMap<TComponent> > ;

		template <typename TComponent>
		using const_iterator = IteratorWrapper < ComponentMap::iterator, const TComponent, ReferenceMap<TComponent>, PointerMap<TComponent> >;

		template <typename TComponent>
		using range = Range < iterator< TComponent > > ;

		template <typename TComponent>
		using const_range = Range < const_iterator< TComponent > >;

		using map_range = Range < ComponentMap::iterator >;

		Component();

		Component(const Component&) = delete;

		virtual ~Component();

		Component& operator=(const Component&) = delete;

		template <typename TComponent, typename... TArgs>
		static TComponent* Create(TArgs&&... arguments);

		template <typename TComponent, typename... TArgs>
		TComponent* AddComponent(TArgs&&... arguments);

		void RemoveComponent();

		template <typename TComponent>
		range<TComponent> GetComponents();

		template <typename TComponent>
		const_range<TComponent> GetComponents() const;

		virtual TypeSet GetTypes() const;

		void Dispose();

	protected:

		virtual void Initialize() = 0;

		virtual void Finalize() = 0;

	private:

		class Arbiter;

		map_range GetComponents(type_index type) const;

		void Setup(Arbiter* arbiter);

		void Setup();

		Arbiter* arbiter_;
		
	};

	////////////////// COMPONENT /////////////////////

	template <typename TComponent, typename... TArgs>
	static TComponent* Component::Create(TArgs&&... arguments){

		TComponent* component = new TComponent(std::forward<TArgs&&>(arguments)...);

		component->Setup();

		return component;

	}

	template <typename TComponent, typename... TArgs>
	TComponent* Component::AddComponent(TArgs&&... arguments){

		TComponent* component = new TComponent(std::forward<TArgs&&>(arguments)...);

		component->Setup(arbiter_);

		return component;

	}

	template <typename TComponent>
	Component::range<TComponent> Component::GetComponents(){

		auto components = GetComponents(type_index(typeid(TComponent)));

		return range<TComponent>(iterator<TComponent>(components.begin(), ReferenceMap<TComponent>(), PointerMap<TComponent>()),
								 iterator<TComponent>(components.end(), ReferenceMap<TComponent>(), PointerMap<TComponent>()));

	}

	template <typename TComponent>
	Component::const_range<TComponent> Component::GetComponents() const{

		auto components = GetComponents(type_index(typeid(TComponent)));

		return const_range<TComponent>(const_iterator<TComponent>(components.begin(), ReferenceMap<TComponent>(), PointerMap<TComponent>()),
									   const_iterator<TComponent>(components.end(), ReferenceMap<TComponent>(), PointerMap<TComponent>()));

	}

	//////////////////////// COMPONENT::REFERENCE MAP /////////////////////

	template <typename TComponent>
	TComponent& Component::ReferenceMap<TComponent>::operator()(Component::ComponentMap::iterator& iterator){

		return *static_cast<TComponent*>(iterator->second);

	}

	//////////////////////// COMPONENT::POINTER MAP ///////////////////////
	
	template <typename TComponent>
	TComponent* Component::PointerMap<TComponent>::operator()(Component::ComponentMap::iterator& iterator){

		return static_cast<TComponent*>(iterator->second);

	}

}