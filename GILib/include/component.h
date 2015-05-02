/// \file component.h
/// \brief Component-based entity classes and methods.
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
#include "observable.h"

using std::type_index;
using std::unordered_multimap;
using std::set;

namespace gi_lib{

	/// \brief Represents a component of a component-based entity.
	/// A component-based entity is an abstract object which exposes different capabilities through components.
	/// These components may be accessed, removed or added at runtime seamlessy.
	/// The entity may have different components of the same type (each of which is a separate object from the others) and may query for components polymorphically.
	/// If an entity has a component of type Derived derived from Base, the entity will responds to both the type Derived and Base.
	/// Components <b>must<\b> be created via Component::Create<TComponent>(...) and destroyed via Component::Dispose().
	/// \auhtor Raffaele D. Facendola.
	class Component{

	public:

		/// \brief Type of the set containing the components.
		using ComponentSet = set < Component* >;

		/// \brief Type of the multimap associating the components to their type.
		using ComponentMap = unordered_multimap < type_index, Component* >;

		/// \brief Type of the type set used to determine the type of a component.
		using TypeSet = set < type_index >;

		/// \brief Functor used to map an entry from the multimap to a component pointer.
		template <typename TComponent>
		struct ComponentMapper{

			/// \brief Maps an iterator to the component pointed by it.
			/// \param pointer Pointer to the object to map.
			/// \return Returns a pointer to the component.
			TComponent* operator()(const ComponentMap::iterator::reference component_pair);

		};

		/// \brief Type of the iterator.
		template <typename TComponent>
		using iterator = IteratorWrapper < ComponentMap::iterator, TComponent, ComponentMapper<TComponent> >;

		/// \brief Type of the constant iterator.
		template <typename TComponent>
		using const_iterator = IteratorWrapper < ComponentMap::iterator, const TComponent, ComponentMapper<TComponent> >;

		/// \brief Range of components.
		template <typename TComponent>
		using range = Range < iterator< TComponent > > ;

		/// \brief Range of constant components.
		template <typename TComponent>
		using const_range = Range < const_iterator< TComponent > >;

		/// \brief Range of components stored inside the component map.
		using map_range = Range < ComponentMap::iterator >;

		/// \brief Arguments of the OnRemoved event.
		struct OnRemovedEventArgs{

			Component* component;	///< \brief Component that has been removed.

		};

		/// \brief Arguments of the OnDisposed event.
		struct OnDisposedEventArgs{

			Component* component;	///< \brief Any component of the entity that has been disposed.

		};


		/// \brief Default constructor.
		Component();

		/// \brief No copy constructor.
		Component(const Component&) = delete;

		/// \brief Virtual destructor for inheritance purposes.
		virtual ~Component();

		/// \brief No assignment operator.
		Component& operator=(const Component&) = delete;

		/// \brief Create and add a new component to the current composite object.
		/// \tparam TComponent Type of the component to create.
		/// \tparam TArgs Types of the arguments to pass to the component's constructor.
		/// \return Returns a pointer to the newly-created component.
		template <typename TComponent, typename... TArgs>
		TComponent* AddComponent(TArgs&&... arguments);

		/// \brief Remove this component from the current composite object.
		/// \remarks Other components are not deleted, meaning that the object may still exist in memory.
		///          If the intention was to delete the entire object, consider using the Dispose method instead.
		/// \remarks If this component was the last one, the composite object is deleted as well.
		/// \see Dispose
		void RemoveComponent();

		/// \brief Return the first component matching a type.
		/// \tparam TComponent Type of component to get.
		/// \return Returns a pointer to the first component that matches the specified type.
		template <typename TComponent>
		TComponent* GetComponent();

		/// \brief Return the first component matching a type.
		/// \tparam TComponent Type of component to get.
		/// \return Returns a pointer to the first component that matches the specified type.
		template <typename TComponent>
		const TComponent* GetComponent() const;

		/// \brief Gets all the component matching a type.
		/// \tparam TComponent Type of component to get.
		/// \return Returns a range of all the stored components that match the specified type.
		template <typename TComponent>
		range<TComponent> GetComponents();

		/// \brief Gets all the component matching a type.
		/// \tparam TComponent Type of component to get.
		/// \return Returns a range of all the stored components that match the specified type.
		template <typename TComponent>
		const_range<TComponent> GetComponents() const;

		/// \brief Get all the component types.
		/// \return Returns a set of types this component can be safely casted to.
		virtual TypeSet GetTypes() const;

		/// \brief Delete this component and every other component.
		void Dispose();

		/// \brief Event triggered when the composite object is being disposed.
		/// The event is ensured to be triggered before the destruction of any component.
		/// \return Returns the event triggered when the composite object is being disposed.
		Observable<OnDisposedEventArgs>& OnDisposed();

		/// \brief Event triggered when this component is being removed from the composite object.
		/// The event is ensured to be triggered before the destrution of this component.
		/// \return Return the event triggered when this component is being removed from the composite object.
		Observable<OnRemovedEventArgs>& OnRemoved();

		/// \brief Create a new component.
		/// \tparam TComponent Type of the component to create.
		/// \tparam TArgs Types of the arguments to pass to the component's constructor.
		/// \return Returns a pointer to the newly-created component.
		template <typename TComponent, typename... TArgs>
		static TComponent* Create(TArgs&&... arguments);

	protected:

		/// \brief Initialize the component.
		/// Use this method for cross-component initialization.
		/// This method is called right after the constructor. AddComponent, RemoveComponent and GetComponents methods are guaranteed to work.
		virtual void Initialize() = 0;

		/// \brief Finalize the component
		/// Use this method for cross-component finalization.
		/// This method is called right before the destructor. AddComponent, RemoveComponent and GetComponents methods are guaranteed to work.
		virtual void Finalize() = 0;

	private:

		class Arbiter;

		/// \brief Return the first component matching a type.
		/// \tparam TComponent Type of component to get.
		/// \return Returns a pointer to the first component that matches the specified type.
		Component* GetComponent(type_index type) const;

		/// \brief Get the components matching a specified type.
		/// \param type Type of the components to get.
		/// \return Returns a range containing all the components matching the given type.
		map_range GetComponents(type_index type) const;
				
		/// \brief Set the arbiter and call the Initialize method.
		/// \param arbiter Arbiter associated to the current composite entity.
		void Setup(Arbiter* arbiter);

		/// \brief Create a new entity and call the Initialize method.
		void Setup();

		Arbiter* arbiter_;								///< \brief Enables intra-component communication.

		Event<OnDisposedEventArgs> on_disposed_event_;	///< \brief Dispose event.

		Event<OnRemovedEventArgs> on_removed_event_;	///< \brief Component remove event.
		
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
	TComponent* Component::GetComponent(){

		return static_cast<TComponent*>(GetComponent(type_index(typeid(TComponent))));
		
	}

	template <typename TComponent>
	const TComponent* Component::GetComponent() const{

		return static_cast<const TComponent*>(GetComponent(type_index(typeid(TComponent))));

	}

	template <typename TComponent>
	Component::range<TComponent> Component::GetComponents(){

		auto components = GetComponents(type_index(typeid(TComponent)));

		return range<TComponent>(iterator<TComponent>(components.begin()),
								 iterator<TComponent>(components.end()));

	}

	template <typename TComponent>
	Component::const_range<TComponent> Component::GetComponents() const{

		auto components = GetComponents(type_index(typeid(TComponent)));

		return const_range<TComponent>(const_iterator<TComponent>(components.begin()),
									   const_iterator<TComponent>(components.end()));

	}

	//////////////////////// COMPONENT::COMPONENT MAPPER ///////////////////////
	
	template <typename TComponent>
	TComponent* Component::ComponentMapper<TComponent>::operator()(const ComponentMap::iterator::reference component_pair){

		return static_cast<TComponent*>(component_pair.second);

	}

}