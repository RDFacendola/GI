/// \file scene.h
/// \brief Defines the base classes used to manage the scene objects and their components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <memory>
#include <set>
#include <initializer_list>

#include "timer.h"
#include "maybe.h"
#include "exceptions.h"

using std::wstring;
using std::map;
using std::unique_ptr;
using std::set;
using std::initializer_list;

namespace gi_lib{
	
	class SceneObject;
	class Component;

	/// \brief A scene object.

	/// A scene object may represents a camera, a light, a model and so on.
	/// Components may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneObject{

	public:
		
		/// \brief Type of the component map.
		typedef map<std::type_index, std::unique_ptr<Component>> ComponentMap;

		/// \brief Type of the tag set.
		typedef set<wstring> TagSet;

		/// \brief Create an unnamed scene object with no tags.
		SceneObject();

		/// \brief Create an unnamed scene object with tags.
		/// \param tags List of tags to associate to this scene object.
		SceneObject(initializer_list<wstring> tags);

		/// \brief Create a named scene object with no tags.
		/// \param name The name of this instance.
		SceneObject(wstring name);

		/// \brief Create a named scene object with tags.
		/// \param name The name of this instance.
		/// \param tags List of tags to associate to this scene object.
		SceneObject(wstring name, initializer_list<wstring> tags);
		
		/// \brief No copy constructor.
		SceneObject(const SceneObject & other) = delete;

		/// \brief No assignment operator.
		SceneObject & operator=(const SceneObject & other) = delete;
		
		/// \brief Move constructor.
		SceneObject(SceneObject && other);

		/// \brief Add a new component to the instance.

		/// If a component of the same type exists, it is overwritten and the previous one is destroyed.
		/// \tparam TComponent Type of the component to add. It must derive from Component.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns a reference to the added component.
		template<typename TComponent, typename... TArgs>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<TComponent&>> AddComponent(TArgs&&... args);

		/// \brief Remove a component by type.
		/// \tparam TComponent Type of the component to remove. It must derive from Component.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, void> RemoveComponent();

		/// \brief Get the component whose type is equal to TComponent.
		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<TComponent &>> GetComponent();

		/// \brief Get the component whose type is equal to TComponent.
		/// \tparam TComponent Type of the component to get. It must derive from Component.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TComponent>
		inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<const TComponent &>> GetComponent() const;

		/// \brief Add a new tag to the scene object.
		/// \param tag The new tag to add.
		inline void AddTag(const wstring & tag);

		/// \brief Remove an existing tag.
		/// \param tag The tag to remove.
		inline void RemoveTag(const wstring & tag);

		/// \brief Check whether the object has a particular tag.
		/// \param tag The tag to match.
		/// \return Returns true if the tag is found, false otherwise.
		inline bool HasTag(const wstring & tag);

		/// \brief Get the scene object's name.

		/// \return Returns the scene object's name.
		inline const wstring & GetName() const;

		/// \brief Updates the enabled components.

		/// \param time The application time
		inline void Update(const Time & time);

	private:

		ComponentMap components_;

		TagSet tags_;

		wstring name_;

	};

	/// \brief Scene object component.
	/// \author Raffaele D. Facendola
	class Component{

		friend class SceneObject;

	public:

		/// \brief Default constructor.
		Component();

		/// \brief No copy constructor.
		Component(const Component & other) = delete;

		/// \brief No assignment operator.
		Component & operator=(const Component & other) = delete;

		virtual ~Component();

		/// \brief Get the component's owner.

		/// \return Returns the component's owner reference.
		SceneObject & GetOwner();

		/// \brief Get the component's owner.

		/// \return Returns the component's owner constant reference.
		const SceneObject & GetOwner() const;

		/// \brief Check wheter this component is enabled.
		/// \return Returns true if the component is enabled, false otherwise.
		bool IsEnabled() const;

		/// \brief Enable or disable the component.
		/// \param enabled Specify true to enable the component, false to disable it.
		void SetEnabled(bool enabled);

	protected:

		/// \brief Update the component.

		/// \param time The current application time.
		virtual void Update(const Time & time) = 0;

	private:

		bool enabled_;

		SceneObject * owner_;

	};

	//

	SceneObject::SceneObject() :
		name_(L""){}

	SceneObject::SceneObject(initializer_list<wstring> tags) :
		name_(L""),
		tags_(tags.begin(), tags.end()){}

	SceneObject::SceneObject(wstring name) :
		name_(std::move(name)){}

	SceneObject::SceneObject(wstring name, initializer_list<wstring> tags) :
		name_(std::move(name)),
		tags_(tags.begin(), tags.end()){}

	SceneObject::SceneObject(SceneObject && other) :
		name_(std::move(other.name_)),
		tags_(std::move(other.tags_)),
		components_(std::move(other.components_)){

		// Change components' ownership
		for (auto & pair : components_){

			pair.second->owner_ = this;

		}

	}

	template<typename TComponent, typename... TArgs>
	inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<TComponent&>> SceneObject::AddComponent(TArgs&&... args){

		auto key = std::type_index(typeid(TComponent));

		auto & ret = (components_[key] = std::make_unique<TComponent>(std::forward<TArgs>(args)...));

		// Set the owner.
		ret->owner_ = this;

		return Maybe<TComponent&>(*(static_cast<TComponent *>(ret.get())));

	}

	template<typename TComponent>
	inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, void> SceneObject::RemoveComponent(){

		auto key = std::type_index(typeid(TComponent));

		components_.erase(key);

	}

	template<typename TComponent>
	inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<TComponent &>> SceneObject::GetComponent(){

		auto key = std::type_index(typeid(TComponent));

		auto it = components_.find(key);

		if (it != components_.end()){

			return Maybe<TComponent &>(*(static_cast<TComponent *>(it->second.get())));

		}
		else{

			return Maybe<TComponent &>();

		}

	}

	template<typename TComponent>
	inline std::enable_if_t<std::is_base_of<Component, TComponent>::value, Maybe<const TComponent &>> SceneObject::GetComponent() const{

		auto key = std::type_index(typeid(TComponent));

		auto it = components_.find(key);

		if (it != components_.end()){

			return const Maybe<const TComponent &>(*(static_cast<TComponent *>(it->second.get())));

		}
		else{

			return const Maybe<const TComponent &>();

		}

	}

	inline void SceneObject::AddTag(const wstring & tag){

		tags_.insert(tag);

	}

	inline void SceneObject::RemoveTag(const wstring & tag){

		tags_.erase(tag);

	}

	inline bool SceneObject::HasTag(const wstring & tag){

		return tags_.find(tag) != tags_.end();

	}

	inline const wstring & SceneObject::GetName() const{

		return name_;

	}

	//

	Component::Component() :
		owner_(nullptr),
		enabled_(true){}

	Component::~Component(){}

	inline SceneObject & Component::GetOwner(){

		return *owner_;

	}

	inline const SceneObject & Component::GetOwner() const{

		return *owner_;

	}

	inline bool Component::IsEnabled() const{

		return enabled_;

	}

	inline void Component::SetEnabled(bool enabled){

		enabled_ = enabled;

	}

}