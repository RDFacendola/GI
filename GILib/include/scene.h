/// \file scene.h
/// \brief Defines the base classes used to manage the scene objects and their components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <set>
#include <initializer_list>

#include "timer.h"
#include "nullable.h"
#include "exceptions.h"

using std::wstring;
using std::map;
using std::unique_ptr;
using std::set;
using std::initializer_list;

namespace gi_lib{
	
	class SceneObject;
	class Component;

	/// \brief Scene object component.
	/// \author Raffaele D. Facendola
	class Component{

		friend class SceneObject;

	public:

		/// \brief Default constructor.
		Component() :
			owner_(nullptr),
			enabled_(true){}

		/// \brief Copy constructor. The owner cannot be copyed in the new instance.
		/// \param other The instance to copy.
		Component(const Component & other){

			enabled_ = other.enabled_;

		}

		/// \brief Assignment operator. The owner cannot be assigned in this instance.
		/// \param other The instance to copy.
		Component & operator=(const Component & other){

			enabled_ = other.enabled_;

			return *this;

		}

		virtual ~Component(){}

		/// \brief Get the component's owner.

		/// \return Returns the component's owner reference.
		inline SceneObject & GetOwner(){

			return *owner_;

		}

		/// \brief Get the component's owner.

		/// \return Returns the component's owner constant reference.
		inline const SceneObject & GetOwner() const{

			return *owner_;

		}

		/// \brief Check wheter this component is enabled.
		/// \return Returns true if the component is enabled, false otherwise.
		inline bool IsEnabled() const{

			return enabled_;

		}

		/// \brief Enable or disable the component.
		/// \param enabled Specify true to enable the component, false to disable it.
		inline void SetEnabled(bool enabled){

			enabled_ = enabled;

		}

	protected:

		/// \brief Update the component.

		/// \param time The current application time.
		virtual void Update(const Timer::Time & time) = 0;

	private:

		bool enabled_;

		SceneObject * owner_;

	};

	/// \brief A scene object.

	/// A scene object may represents a camera, a light, a model and so on.
	/// Components may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneObject{

	public:
		
		/// \brief Type of the component map.
		typedef map<size_t, std::unique_ptr<Component>> ComponentMap;

		/// \brief Type of the tag set.
		typedef set<wstring> TagSet;

		/// \brief Create an unnamed scene object with no tags.
		SceneObject() :
			name_(L""){}

		/// \brief Create an unnamed scene object with tags.
		/// \param tags List of tags to associate to this scene object.
		SceneObject(initializer_list<wstring> tags) :
			name_(L""),
			tags_(tags.begin(), tags.end()){}

		/// \brief Create a named scene object with no tags.
		/// \param name The name of this instance.
		SceneObject(wstring name) :
			name_(std::move(name)){}

		/// \brief Create a named scene object with tags.
		/// \param name The name of this instance.
		/// \param tags List of tags to associate to this scene object.
		SceneObject(wstring name, initializer_list<wstring> tags) :
			name_(std::move(name)),
			tags_(tags.begin(), tags.end()){}

		/// TODO copy-ctor, assignment & move (+swap)

		/// \brief Add a new component to the instance.

		/// If a component of the same type exists, it is overwritten and the previous one is destroyed.
		/// \tparam TComponent Type of the component to add. It must derive from Component.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns a reference to the created component.
		template<typename TComponent, typename... TArgs>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Nullable<TComponent>> AddComponent(TArgs&&... args){
			
			auto key = typeid(TComponent).hash_code();

			auto & ret =(components_[key] = std::make_unique<TComponent>(std::forward<TArgs>(args)...));

			// Set the owner.
			ret->owner_ = this;

			return make_nullable(*(static_cast<TComponent *>(ret.get())));

		}

		/// \brief Remove a component by type.

		/// \tparam TComponent Type of the component to remove. It must derive from Component.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, void> RemoveComponent(){

			auto key = typeid(TComponent).hash_code();

			components_.erase(key);

		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Return a weak pointer to the component found. Returns an empty pointer if no component was found.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Nullable<TComponent>> GetComponent(){

			auto key = typeid(TComponent).hash_code();

			auto it = components_.find(key);

			if (it != components_.end()){

				return make_nullable(*(static_cast<TComponent *>(it->second.get())));

			}else{

				return Nullable<TComponent>();

			}
			
		}

		/// \brief Get the first component deriving from TComponent.

		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Return a weak pointer to the component found. Returns an empty pointer if no component was found.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Nullable<const TComponent>> GetComponent() const{

			auto key = typeid(TComponent).hash_code();

			auto it = components_.find(key);

			if (it != components_.end()){

				return make_nullable(*(static_cast<const TComponent *>(it->second.get())));

			}
			else{

				return Nullable<const TComponent>();

			}

		}

		/// \brief Add a new tag to the scene object.
		/// \param tag The new tag to add.
		inline void AddTag(const wstring & tag){

			tags_.insert(tag);

		}

		/// \brief Remove an existing tag.
		/// \param tag The tag to remove.
		inline void RemoveTag(const wstring & tag){

			tags_.erase(tag);

		}

		/// \brief Check whether the object has a particular tag.
		/// \param tag The tag to match.
		/// \return Returns true if the tag is found, false otherwise.
		inline bool HasTag(const wstring & tag){

			return tags_.find(tag) != tags_.end();

		}

		/// \brief Get the scene object's name.

		/// \return Returns the scene object's name.
		inline const wstring & GetName() const{

			return name_;

		}

		/// \brief Updates the enabled components.

		/// \param time The application time
		inline void Update(const Timer::Time & time){

			for (auto & it : components_){

				if (it.second->IsEnabled()){

					it.second->Update(time);

				}
				
			}

		}

	private:

		ComponentMap components_;

		TagSet tags_;

		wstring name_;

	};
	
}