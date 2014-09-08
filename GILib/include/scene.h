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
#include "unique.h"

using std::wstring;
using std::map;
using std::unique_ptr;
using std::set;
using std::initializer_list;

namespace gi_lib{
	
	class SceneNode;

	/// \brief Represents an entire scene.
	class Scene{

	public:

		/// \brief Create a new scene.
		Scene();

		/// \brief No assignment operator.
		Scene & operator=(const Scene &) = delete;

		/// \brief Create a new scene node.

		/// The scene node is attached to the root by default.
		/// \param name The name of the node
		/// \param local_transform The transform of the node in local space.
		/// \param tags Tags associated to the node.
		/// \return Returns a reference to the new node.
		template <typename... TArgs>
		SceneNode & CreateNode(TArgs&&... args);

		/// \brief Destroy an existing node.

		/// \param node The node to destroy. <b>Do not attempt to use the node afterwards!.</b>
		void DestroyNode(SceneNode & node);

		/// \brief Update the entire scene.
		/// \param time The current application time.
		void Update(const Time & time);

		/// \brief The provided node becomes a transform root.

		/// The node is detached from the previous parent, if any.
		/// \param node The node to attach to the scene root.
		void SetRoot(SceneNode & node);

	private:

		/// \brief Type of the node map.
		using NodeMap = map<Unique<SceneNode>, unique_ptr<SceneNode>>;

		unique_ptr<SceneNode> root_;	// Root of the scene

		NodeMap nodes_;					// Nodes inside the scene

	};

	/// \brief Represents a scene node.

	/// A node may represents a camera, a light, a model and so on.
	/// NodeComponents may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneNode{

		friend class Scene;

	public:
		
		/// \brief Create a default scene node.

		/// The node won't have any name or tags and its local transformation matrix will be the identity matrix.
		/// \param scene The scene who owns this node.
		SceneNode(Scene & scene);

		/// \brief Create a scene node.
		/// \param scene The scene who owns this node.
		/// \param name Name of the scene node. It may not be unique.
		/// \param local_transform Affine transformation in local space.
		/// \param tags List of tags associated to the scene node.
		SceneNode(Scene & scene, const wstring & name, const Affine3f & local_transform, initializer_list<wstring> tags = {});
		
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

		/// \brief Assign this instance transform to another parent.
		/// \param parent The node who contains the new parent transform.
		void SetParent(SceneNode & parent);

		/// \brief Check whether this node is a root.
		/// \return Returns true if the node is a root, false otherwise.
		bool IsRoot();

		/// \brief Get the unique ID identifying this scene node.
		/// \return Returns an unique object which is guaranteed to be unique among other scene nodes.
		const Unique<SceneNode> & GetUniqueID() const;
		
		/// \brief Get the scene this node belongs to.
		/// \return Returns the scene this node belongs to.
		Scene & GetScene();

		/// \brief Get the scene this node belongs to.
		/// \return Returns the scene this node belongs to.
		const Scene & GetScene() const;

		/// \brief Updates the enabled components.

		/// \param time The application time
		void Update(const Time & time);

	private:



		/// \brief Type of the component map.
		using ComponentMap = map<std::type_index, std::unique_ptr<NodeComponent>>;

		/// \brief Type of the tag set.
		using TagSet = set<wstring>;

		/// \brief Updates the hierarchy of scene nodes.
		void UpdateHierarchy(const Time & time);

		ComponentMap components_;

		Transform & transform_;

		Scene & scene_;

		TagSet tags_;

		wstring name_;

		Unique<SceneNode> unique_;

	};

	//

	template <typename... TArgs>
	SceneNode & Scene::CreateNode(TArgs&&... args){

		auto node = std::make_unique<SceneNode>(*this, std::forward<TArgs>(args)...);

		SetRoot(*node);

		auto key = node->GetUniqueID();

		auto & ret = (nodes_[key] = std::move(node));

		return *ret;

	}

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

	inline const Unique<SceneNode> & SceneNode::GetUniqueID() const{

		return unique_;

	}

	inline void SceneNode::SetParent(SceneNode & parent){

		transform_.SetParent(parent.GetTransform());

	}

	inline bool SceneNode::IsRoot(){

		return transform_.IsRoot();

	}

	Scene & SceneNode::GetScene(){

		return scene_;

	}

	const Scene & SceneNode::GetScene() const{

		return scene_;

	}

}