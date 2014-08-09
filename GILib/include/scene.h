/// \file scene.h
/// \brief Defines the base classes used to manage the scene objects and their components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <unordered_map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "iterator.h"
#include "timer.h"

using std::wstring;
using std::unordered_multimap;
using std::shared_ptr;
using std::weak_ptr;

namespace gi_lib{
	
	/// \brief A scene object.

	/// A scene object may represents a camera, a light, a model and so on.
	/// Components may be plugged to customize its behaviour
	class SceneObject{

	public:
		
		class Component;

		/// \brief Type of the component map
		typedef unordered_multimap<size_t, shared_ptr<Component>> ComponentMap;

		/// \brief Scene object component.
		class Component{

		public:

			virtual ~Component(){}

			/// \brief Get the component's owner.

			/// \return Returns the component's owner reference.
			inline SceneObject & GetOwner(){

				return *scene_object_;

			}

			/// \brief Get the component's owner.

			/// \return Returns the component's owner constant reference.
			inline const SceneObject & GetOwner() const{

				return *scene_object_;

			}

			/// \brief Update the component.

			/// \param time The current application time.
			virtual void Update(const Timer::Time & time) = 0;

		private:

			/// \brief Set the component's owner.

			/// \param scene_object The owner of this instance.
			inline void SetOwner(SceneObject & scene_object){

				scene_object_ = &scene_object;

			}

			SceneObject * scene_object_;

		};

		/// \brief Iterator used to iterate through scene object's components.

		/// \tparam TComponent Type of the component this iterator points to.
		template <typename TComponent>
		class ComponentIterator{

		public:

			ComponentIterator(SceneObject::ComponentMap::iterator iterator) :
				iterator_(iterator){};

			// Dereferencing

			TComponent & operator*(){

				return *static_cast<TComponent *>((*iterator_).second);

			}

			TComponent * operator->(){

				return static_cast<TComponent *>((*iterator_).second);

			}

			// Equality

			inline bool operator==(const ComponentIterator & other) const{

				return iterator_ == other.iterator_;

			}

			inline bool operator==(const SceneObject::ComponentMap::iterator & other) const{

				return iterator_ == other;

			}

			inline bool operator!=(const ComponentIterator & other) const{

				return iterator_ != other.iterator_;

			}

			inline bool operator!=(const SceneObject::ComponentMap::iterator & other) const{

				return iterator_ != other;

			}

			// Advance

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

		/// \brief Constant iterator used to iterate through scene object's components.

		/// \tparam TComponent Type of the component this iterator points to.
		template <typename TComponent>
		class ComponentConstIterator{

		public:

			ComponentConstIterator(SceneObject::ComponentMap::const_iterator iterator) :
				iterator_(iterator){};

			//Dereferencing

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

			inline bool operator==(const SceneObject::ComponentMap::const_iterator & other) const{

				return iterator_ == other;

			}

			inline bool operator!=(const ComponentConstIterator & other) const{

				return iterator_ != other.iterator_;

			}

			inline bool operator!=(const SceneObject::ComponentMap::const_iterator & other) const{

				return iterator_ != other;

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

			SceneObject::ComponentMap::const_iterator iterator_;

		};

		/// \brief Create an unnamed scene object.
		SceneObject() :
			name_(L""){}

		/// \brief Create a named scene object.

		/// \param name The name of this instance.
		SceneObject(wstring name) :
			name_(name){}

		/// \brief Add a new component to the instance.

		/// The component to add is created inside the method as its lifecycle is entirely managed by its owner.
		/// \tparam TComponent Type of the component to add. It must derive from Component.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns an iterator pointing to the new component.
		template<typename TComponent, typename... TArgs>
		inline ComponentIterator<TComponent> AddComponent(TArgs&&... args){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto component = std::make_shared<TComponent>(std::forward<TArgs>(args)...);

			///Set the owner of the component
			component->SetOwner(*this);

			return ComponentIterator<TComponent>(components_.insert(ComponentMap::value_type(typeid(TComponent).hash_code(),
																							 component)));

		}

		/// /brief Remove the component pointed by the iterator.

		/// The component is destroyed and its destructor gets called accordingly.
		/// The specified iterator is invalidated.
		/// \tparam TComponent Type of the component to remove. It must derive from Component. 
		/// \param iterator Iterator pointing to the component to be removed.
		template<typename TComponent>
		void RemoveComponent(ComponentIterator<TComponent> & iterator){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			for (auto it = components_.find(typeid(TComponent).hash_code()); it != components_.end();){

				if (it == iterator){

					components_.erase(it++);

					iterator = components_.end();

					return;

				}else{

					++it;

				}

			}

		}

		/// \brief Remove all the components deriving from TComponent.

		/// The removed components are destroyed and their destructors get called accordingly.
		/// \tparam TComponent Type of the components to remove. It must derive from Component.
		template<typename TComponent>
		inline void RemoveComponents(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			components_.erase(typeid(TComponent).hash_code());

		}

		/// \brief Get all the components deriving from TComponent.

		/// \tparam TComponent Type of the components to get. It must derive from Component.
		/// \return Returns a range containing all the components that derive from TComponent.
		template<typename TComponent>
		inline Range<TComponent, ComponentIterator<TComponent>> GetComponents(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto range = components_.equal_range(typeid(TComponent).hash_code());

			return Range<TComponent, ComponentIterator<TComponent>>(ComponentIterator<TComponent>(range.first),
																	ComponentIterator<TComponent>(range.second));

		}

		/// \brief Get all the components deriving from TComponent.

		/// \tparam TComponent Type of the components to get. It must derive from Component.
		/// \return Returns a range containing all the components that derive from TComponent.
		template<typename TComponent>
		inline Range<TComponent, ComponentConstIterator<TComponent>> GetComponents() const{

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto range = components_.equal_range(typeid(TComponent).hash_code());

			return Range<TComponent, ComponentConstIterator<TComponent>>(ComponentConstIterator<TComponent>(range.first),
				ComponentConstIterator<TComponent>(range.second));

		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Returns an iterator pointing to the first occurence found if any. Returns an iterator pointing to an element past the end otherwise.
		template<typename TComponent>
		inline ComponentIterator<TComponent> GetComponent(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			return ComponentIterator<TComponent>(components_.find(typeid(TComponent).hash_code()));

		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Returns a constant iterator pointing to the first occurence found if any. Returns an iterator pointing to an element past the end otherwise.
		template<typename TComponent>
		inline ComponentConstIterator<TComponent> GetComponent() const{

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			return ComponentConstIterator<TComponent>(components_.find(typeid(TComponent).hash_code()));

		}

		/// \brief Get an iterator pointing to a component past the end.

		/// \return Return an iterator pointing to a component past the end.
		template<typename TComponent>
		inline ComponentIterator<TComponent> GetEnd(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			return ComponentIterator<TComponent>(components_.end());

		}

		/// \brief Get a constant iterator pointing to a component past the end.

		/// \return Return a constant iterator pointing to a component past the end.
		template<typename TComponent>
		inline ComponentConstIterator<TComponent> GetEnd() const{

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			return ComponentConstIterator<TComponent>(components_.end());

		}

		/// \brief Get the scene object's name.

		/// \return Returns the scene object's name.
		inline const wstring & GetName() const{

			return name_;

		}

		/// \brief Updates the components of this scene object.

		/// \param time The application time
		inline void Update(const Timer::Time & time){

			for (auto & it : components_){

				it.second->Update(time);

			}

		}

	private:

		ComponentMap components_;

		wstring name_;

	};
	
}