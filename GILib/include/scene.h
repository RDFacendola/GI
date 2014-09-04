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
	
	/// \brief A scene node.

	/// A scene object may represents a camera, a light, a model and so on.
	/// NodeComponents may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneNode{

	public:
		
		/// \brief Type of the component map.
		typedef map<std::type_index, std::unique_ptr<NodeComponent>> NodeComponentMap;

		/// \brief Type of the tag set.
		typedef set<wstring> TagSet;

		/// \brief Create an unnamed scene object with no tags.
		SceneNode();

		/// \brief Create an unnamed scene object with tags.
		/// \param tags List of tags to associate to this scene object.
		SceneNode(initializer_list<wstring> tags);

		/// \brief Create a named scene object with no tags.
		/// \param name The name of this instance.
		SceneNode(wstring name);

		/// \brief Create a named scene object with tags.
		/// \param name The name of this instance.
		/// \param tags List of tags to associate to this scene object.
		SceneNode(wstring name, initializer_list<wstring> tags);
		
		/// \brief No copy constructor.
		SceneNode(const SceneNode & other) = delete;

		/// \brief No assignment operator.
		SceneNode & operator=(const SceneNode & other) = delete;
		
		/// \brief Move constructor.
		SceneNode(SceneNode && other);

		/// \brief Add a new component to the instance.

		/// If a component of the same type exists, it is overwritten and the previous one is destroyed.
		/// \tparam TNodeComponent Type of the component to add. It must derive from NodeComponent.
		/// \tparam TArgs Type of the arguments that will be passed to the component during its creation.
		/// \param args Arguments that will be passed to the component during its creation.
		/// \return Returns a reference to the added component.
		template<typename TNodeComponent, typename... TArgs>
		inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent&>> AddNodeComponent(TArgs&&... args);

		/// \brief Remove a component by type.
		/// \tparam TNodeComponent Type of the component to remove. It must derive from NodeComponent.
		template<typename TNodeComponent>
		inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, void> RemoveNodeComponent();

		/// \brief Get the component whose type is equal to TNodeComponent.
		/// \tparam TNodeComponent Type of the component to get. It must derive from NodeComponent.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TNodeComponent>
		inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent &>> GetNodeComponent();

		/// \brief Get the component whose type is equal to TNodeComponent.
		/// \tparam TNodeComponent Type of the component to get. It must derive from NodeComponent.
		/// \return Returns a reference to the found object, if any. Returns an empty reference otherwise.
		template<typename TNodeComponent>
		inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<const TNodeComponent &>> GetNodeComponent() const;

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

		NodeComponentMap components_;

		TagSet tags_;

		wstring name_;

	};

	//

	SceneNode::SceneNode() :
		name_(L""){}

	SceneNode::SceneNode(initializer_list<wstring> tags) :
		name_(L""),
		tags_(tags.begin(), tags.end()){}

	SceneNode::SceneNode(wstring name) :
		name_(std::move(name)){}

	SceneNode::SceneNode(wstring name, initializer_list<wstring> tags) :
		name_(std::move(name)),
		tags_(tags.begin(), tags.end()){}

	SceneNode::SceneNode(SceneNode && other) :
		name_(std::move(other.name_)),
		tags_(std::move(other.tags_)),
		components_(std::move(other.components_)){

		// Change components' ownership
		for (auto & pair : components_){

			pair.second->owner_ = this;

		}

	}

	template<typename TNodeComponent, typename... TArgs>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent&>> SceneNode::AddNodeComponent(TArgs&&... args){

		auto key = std::type_index(typeid(TNodeComponent));

		auto & ret = (components_[key] = std::make_unique<TNodeComponent>(std::forward<TArgs>(args)...));

		// Set the owner.
		ret->owner_ = this;

		return Maybe<TNodeComponent&>(*(static_cast<TNodeComponent *>(ret.get())));

	}

	template<typename TNodeComponent>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, void> SceneNode::RemoveNodeComponent(){

		auto key = std::type_index(typeid(TNodeComponent));

		components_.erase(key);

	}

	template<typename TNodeComponent>
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<TNodeComponent &>> SceneNode::GetNodeComponent(){

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
	inline std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, Maybe<const TNodeComponent &>> SceneNode::GetNodeComponent() const{

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

}