/// \file scene.h
/// \brief Defines the base classes used to manage the scene.
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

#include "components.h"
#include "timer.h"
#include "maybe.h"
#include "exceptions.h"

using std::wstring;
using std::map;
using std::unique_ptr;
using std::set;
using std::initializer_list;

namespace gi_lib{
	
	/// \brief Represents an entire scene.
	class Scene{

	};

	/// \brief Represents a scene node.

	/// A node may represents a camera, a light, a model and so on.
	/// NodeComponents may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneNode{

		friend class Scene;

	public:
		
		/// \brief Type of the component map.
		typedef map<std::type_index, std::unique_ptr<NodeComponent>> ComponentMap;

		/// \brief Type of the tag set.
		typedef set<wstring> TagSet;

		/// \brief Create a default scene node.

		/// The node won't have any name or tags and its local transformation matrix will be the identity matrix.
		SceneNode();

		/// \brief Create a scene node.
		/// \param name Name of the scene node. It may not be unique.
		/// \param local_transform Affine transformation in local space.
		/// \param tags List of tags associated to the scene node.
		SceneNode(const wstring & name, const Affine3f & local_transform, initializer_list<wstring> tags);
		
		/// \brief Move constructor.
		SceneNode(SceneNode && other);

		/// \brief No copy constructor.
		SceneNode(const SceneNode & other) = delete;

		/// \brief No assignment operator.
		SceneNode & operator=(const SceneNode & other) = delete;
		
		/// \brief Add a new component to the instance.

		/// If a component of the same type exists, it is overwritten and the previous one is destroyed.
		/// \tparam TNodeComponent Type of the component to add. It must derive from NodeComponent.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns a reference to the added component.
		template<typename TNodeComponent, typename... TArgs>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent&>> Add(TArgs&&... args);

		/// \brief Remove a component by type.
		/// \tparam TNodeComponent Type of the component to remove. It must derive from NodeComponent.
		template<typename TNodeComponent>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, void> Remove();

		/// \brief Get the component whose type is equal to TNodeComponent.
		/// \tparam TNodeComponent Type of the component to get. It must derive from NodeComponent.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TNodeComponent>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent &>> Get();

		/// \brief Get the component whose type is equal to TNodeComponent.
		/// \tparam TNodeComponent Type of the component to get. It must derive from NodeComponent.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TNodeComponent>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<const TNodeComponent &>> Get() const;

		/// \brief Add a new tag to the scene object.
		/// \param tag The new tag to add.
		void AddTag(const wstring & tag);

		/// \brief Remove an existing tag.
		/// \param tag The tag to remove.
		void RemoveTag(const wstring & tag);

		/// \brief Check whether the object has a particular tag.
		/// \param tag The tag to match.
		/// \return Returns true if the tag is found, false otherwise.
		bool HasTag(const wstring & tag);

		/// \brief Get the scene object's name.

		/// \return Returns the scene object's name.
		const wstring & GetName() const;

		/// \brief Get the transform component of the node.
		/// \return Returns a reference to the transform component.
		Transform & GetTransform();

		/// \brief Get the transform component of the node.
		/// \return Returns a constant reference to the transform component.
		const Transform & GetTransform() const;
		
		/// \brief Updates the enabled components.

		/// \param time The application time
		void Update(const Time & time);

	private:

		/// \brief Updates the hierarchy of scene nodes.
		void UpdateHierarchy(const Time & time);

		ComponentMap components_;

		Transform & transform_;

		TagSet tags_;

		wstring name_;

	};

	//

	template<typename TNodeComponent, typename... TArgs>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent&>> SceneNode::Add(TArgs&&... args){

		auto key = std::type_index(typeid(TNodeComponent));

		auto & ret = (components_[key] = std::make_unique<TNodeComponent>(std::forward<TArgs>(args)...));

		// Set the owner.
		ret->owner_ = this;

		return Maybe<TNodeComponent&>(*(static_cast<TNodeComponent *>(ret.get())));

	}

	template<typename TNodeComponent>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, void> SceneNode::Remove(){

		auto key = std::type_index(typeid(TNodeComponent));

		components_.erase(key);

	}

	template<typename TNodeComponent>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent &>> SceneNode::Get(){

		auto key = std::type_index(typeid(TNodeComponent));

		auto it = components_.find(key);

		if (it != components_.end()){

			return Maybe<TNodeComponent &>(*(static_cast<TNodeComponent *>(it->second.get())));

		}
		else{

			return Maybe<TNodeComponent &>();

		}

	}

	template<typename TNodeComponent>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<const TNodeComponent &>> SceneNode::Get() const{

		auto key = std::type_index(typeid(TNodeComponent));

		auto it = components_.find(key);

		if (it != components_.end()){

			return const Maybe<const TNodeComponent &>(*(static_cast<TNodeComponent *>(it->second.get())));

		}
		else{

			return const Maybe<const TNodeComponent &>();

		}

	}

	inline void SceneNode::AddTag(const wstring & tag){

		tags_.insert(tag);

	}

	inline void SceneNode::RemoveTag(const wstring & tag){

		tags_.erase(tag);

	}

	inline bool SceneNode::HasTag(const wstring & tag){

		return tags_.find(tag) != tags_.end();

	}

	inline const wstring & SceneNode::GetName() const{

		return name_;

	}

	inline Transform & SceneNode::GetTransform(){

		return transform_;

	}

	inline const Transform & SceneNode::GetTransform() const{

		return transform_;

	}

}