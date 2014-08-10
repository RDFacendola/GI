/// \file scene.h
/// \brief Defines the base classes used to manage the scene objects and their components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <algorithm>
#include <iterator>

#include "iterator.h"
#include "timer.h"

using std::wstring;
using std::unordered_multimap;
using std::shared_ptr;
using std::weak_ptr;
using std::vector;

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

			friend class SceneObject;

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

		protected:

			/// \brief Update the component.

			/// \param time The current application time.
			virtual void Update(const Timer::Time & time) = 0;

		private:

			/// \brief Set the component's owner.

			/// \param scene_object Pointer to the owner of this instance.
			inline void SetOwner(SceneObject * scene_object){

				scene_object_ = scene_object;

			}

			SceneObject * scene_object_;

		};

		/// \brief Create an unnamed scene object.
		SceneObject() :
			name_(L""){}

		/// \brief Create a named scene object.

		/// \param name The name of this instance.
		SceneObject(wstring name) :
			name_(std::move(name)){}

		/// \brief Add a new component to the instance.

		/// SceneObject owns the created component.
		/// \tparam TComponent Type of the component to add. It must derive from Component.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns a weak pointer to the created component.
		template<typename TComponent, typename... TArgs>
		inline weak_ptr<TComponent> AddComponent(TArgs&&... args){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto component = std::make_shared<TComponent>(std::forward<TArgs>(args)...);

			///Set the owner of the component
			component->SetOwner(this);

			components_.insert(ComponentMap::value_type(typeid(TComponent).hash_code(),
														component));

			return component;

		}

		/// /brief Remove a given component.

		/// The component is destroyed and its destructor called accordingly.
		/// \tparam TComponent Type of the component to remove. It must derive from Component. 
		/// \param component Weak pointer to the component to delete.
		template<typename TComponent>
		void RemoveComponent(weak_ptr<TComponent> & component){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			if (auto temp_component = component.lock()){

				// Null out the owner
				temp_component->SetOwner(nullptr);

				for (auto it = components_.find(typeid(TComponent).hash_code()); it != components_.end();){

					if (it->second == temp_component){

						components_.erase(it++);
						
						return;

					}
					else{

						++it;

					}

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
		/// \return Returns a vector containing weak pointer to the components that derive from TComponent.
		template<typename TComponent>
		inline vector<weak_ptr<TComponent>> GetComponents(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto range = components_.equal_range(typeid(TComponent).hash_code());

			auto components = vector<weak_ptr<TComponent>>(std::distance(range.first, range.second));

			std::transform(range.first,
						   range.second,
						   components.begin(),
						   [](const ComponentMap::value_type & value){ return std::static_pointer_cast<TComponent>(value.second); });

			return components;

		}

		/// \brief Get all the components deriving from TComponent.

		/// \tparam TComponent Type of the components to get. It must derive from Component.
		/// \return Returns a vector containing weak pointer to the components that derive from TComponent.
		template<typename TComponent>
		inline vector<weak_ptr<const TComponent>> GetComponents() const{

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto range = components_.equal_range(typeid(TComponent).hash_code());

			auto components = vector<weak_ptr<const TComponent>>(std::distance(range.first, range.second));

			std::transform(range.first,
						   range.second,
						   components.begin(),
						   [](const ComponentMap::value_type & value){ return std::static_pointer_cast<const TComponent>(value.second); });

			return components;

		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Return a weak pointer to the component found. Returns an empty pointer if no component was found.
		template<typename TComponent>
		inline weak_ptr<TComponent> GetComponent(){

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto it = components_.find(typeid(TComponent).hash_code());

			if (it != components_.end()){

				return std::static_pointer_cast<TComponent>(it->second);

			}else{

				return weak_ptr<TComponent>();

			}
			
		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Return a weak pointer to the component found. Returns an empty pointer if no component was found.
		template<typename TComponent>
		inline weak_ptr<const TComponent> GetComponent() const{

			//Ensures that TComponent is derived from Component at compile time
			static_assert(typename std::is_base_of<Component, TComponent>::value, "TComponent must inherit from Component");

			auto it = components_.find(typeid(TComponent).hash_code());

			if (it != components_.end()){

				return std::static_pointer_cast<const TComponent>(it->second);

			}
			else{

				return weak_ptr<const TComponent>();

			}

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