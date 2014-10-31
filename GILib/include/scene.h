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
#include <functional>
#include <iterator>
#include <numeric>

#include "components.h"
#include "timer.h"
#include "maybe.h"
#include "exceptions.h"
#include "unique.h"

using std::wstring;
using std::multimap;
using std::unique_ptr;
using std::set;
using std::initializer_list;
using std::reference_wrapper;
using std::iterator;

namespace gi_lib{
	
	class Scene;
	class SceneNode;
	
	/// \brief Used to build a scene node
	class SceneNodeBuilder{

	public:

		/// \brief Create a new scene node builder.
		/// \param parent The node's parent
		SceneNodeBuilder(SceneNode & parent);

		/// \brief Set the node's parent.
		/// \param parent The node's parent.
		void SetParent(SceneNode & parent);

		/// \brief Set the node's name.
		/// \param name The node's name.
		void SetName(const wstring & name);

		/// \brief Set the node's position.
		/// \param position The node's position.
		void SetPosition(const Translation3f & position);

		/// \brief Set the node's rotation.
		/// \param rotation The node's rotation.
		void SetRotation(const Quaternionf & rotation);

		/// \brief Set the node's scaling.
		/// \param scaling The node's scaling.
		void SetScaling(const AlignedScaling3f & scaling);

		/// \brief Set the node's tags.
		/// \param tags The node's tags.
		void SetTags(initializer_list<wstring> tags);

		/// \brief Build a new scene node.
		/// \return Returns a new scene node built according to the previous parameters set.
		SceneNode & Build() const;

	private:

		SceneNode * parent_;

		wstring name_ = L"";

		Translation3f position_ = Translation3f(Vector3f::Zero());

		Quaternionf rotation_ = Quaternionf::Identity();

		AlignedScaling3f scaling_ = AlignedScaling3f(Vector3f::Ones());

		initializer_list<wstring> tags_;

	};

	/// \brief Represents a scene node.

	/// A node may represents a camera, a light, a model and so on.
	/// NodeComponents may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneNode{

		friend class Scene;

	public:
		
		/// \brief Type of the component list.
		using ComponentList = vector < shared_ptr<NodeComponent> >;

		/// \brief Type of the children list.
		using ChildrenList = vector < SceneNode* > ;

		/// \brief Create a root node.
		SceneNode();

		/// \brief Create a scene node.
		/// \param name Name of the scene node. It may not be unique.
		/// \param position Position of the node in local space.
		/// \param rotation Rotation of the node in local space.
		/// \param scaling Scaling of the node in local space.
		/// \param tags List of tags associated to the scene node.
		SceneNode(const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags);

		/// \brief Destroy the current scene node and all the children.
		~SceneNode();

		/// \brief No copy constructor.
		SceneNode(const SceneNode & other) = delete;

		/// \brief No assignment operator.
		SceneNode & operator=(const SceneNode & other) = delete;
		
		/// \brief Add a new component.
		/// \tparam TNodeComponent Type of the component to add. It must derive from NodeComponent.
		/// \param args Arguments to pass to the component's constructor.
		template<typename TNodeComponent, typename... TArgs>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, shared_ptr<TNodeComponent>> AddComponent(TArgs&&... args);

		/// \brief Remove all the component whose dynamic type matches the specified one.

		/// \tparam Dynamic type of the components to remove. It must derive from NodeComponent.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		void RemoveComponents();

		/// \brief Remove the specified component.
		/// \tparam TNodeComponent Type of the component to remove. It must derive from NodeComponent.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		void RemoveComponent(typename shared_ptr<TNodeComponent> component);

		/// \brief Get the first component whose dynamic type matched the specified one.
		/// \tparam TNodeComponent Dynamic type of the component to get. It must derive from NodeComponent.
		/// \return Returns a pointer to the first component matching the specified type. Returns null if no component could be found.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename shared_ptr<TNodeComponent> GetComponent();

		/// \brief Get the first component whose dynamic type matched the specified one.
		/// \tparam TNodeComponent Dynamic type of the component to get. It must derive from NodeComponent.
		/// \return Returns a pointer to the first component matching the specified type. Returns null if no component could be found.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename shared_ptr<const TNodeComponent> GetComponent() const;

		/// \brief Get the list of all components whose dynamic type match the specified one.
		/// \tparam TNodeComponent Dynamic type of the components to get. It must derive from NodeComponent.
		/// \return Returns the list of all components matching the specified type.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename vector < shared_ptr<TNodeComponent> > GetComponents();

		/// \brief Get the list of all components whose dynamic type match the specified one.
		/// \tparam TNodeComponent Dynamic type of the components to get. It must derive from NodeComponent.
		/// \return Returns the list of all components matching the specified type.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename vector < shared_ptr<const TNodeComponent> > GetComponents() const;

		/// \brief Updates the enabled components.

		/// \param time The application time.
		void Update(const Time & time);

		/// \brief Post updats the node.

		/// \param time The application time.
		void PostUpdate(const Time & time);

		// Transformation and hierarchy
		
		/// \brief Get the position component.
		/// \return Returns the position component.
		const Translation3f & GetPosition() const;

		/// \brief Set the position component.
		/// \param position The new position.
		void SetPosition(const Translation3f & position);

		/// \brief Get the rotation component.
		/// \return Returns the rotation component.
		const Quaternionf & GetRotation() const;

		/// \brief Set the rotation component.
		/// \param rotation The new rotation.
		void SetRotation(const Quaternionf & rotation);

		/// \brief Get the scaling component.
		/// \return Returns the scaling component.
		const AlignedScaling3f & GetScaling() const;

		/// \brief Set the scaling component.
		/// \param scaling The new scaling.
		void SetScaling(const AlignedScaling3f & scaling);

		/// \brief Get the local transfom.

		/// The local transformation matrix applies the scaling first, the rotation second and the translation last.
		/// \return Returns the local transform matrix.
		const Affine3f & GetLocalTransform();

		/// \brief Get the global transfom.
		/// \return Returns the global transform matrix.
		const Affine3f & GetWorldTransform();

		/// \brief Get this node's parent.
		/// \return Returns a reference to the node's parent. If the node is actually the root of the scene, it returns a reference to this object.
		SceneNode & GetParent();
		
		/// \brief Get this node's parent.
		/// \return Returns a reference to the node's parent. If the node is actually the root of the scene, it returns a reference to this object.
		const SceneNode & GetParent() const;

		/// \brief Set this node's parent.

		/// The specified parent will take node's ownership and will destroy this node upon its own destruction.
		/// You may not change a scene root`s parent or create a new root (by setting the parent parameter to *this, for example)
		/// You amy not create cycles inside the scene graph!
		/// \param parent The new parent.
		void SetParent(SceneNode & parent);

		/// \brief Remove this node from the parent's children.
		void ResetParent();

		/// \brief Check whether this node is a root.
		/// \return Returns true if the node is a root, false otherwise.
		bool IsRoot() const;

		/// \brief Get the builder used to build child nodes.
		/// \return Return a builder whose parent is the current instance.
		SceneNodeBuilder GetBuilder();

		/// \brief Get the first child.
		/// \return Returns an iterator to the first child
		ChildrenList::iterator ChildrenBegin();

		/// \brief Get one element past the last child.
		/// \return Returns an iterator to the first child past the last one. Do not dereference this iterator!
		ChildrenList::iterator ChildrenEnd();

		/// \brief Get the first child.
		/// \return Returns an iterator to the first child
		ChildrenList::const_iterator ChildrenBegin() const;

		/// \brief Get one element past the last child.
		/// \return Returns an iterator to the first child past the last one. Do not dereference this iterator!
		ChildrenList::const_iterator ChildrenEnd() const;

		/// \brief Get the children count.
		/// \return Returns the children count
		unsigned int GetChildrenCount() const;

		/// \brief Find all the nodes matching the specified name.
		/// \param name The name to find.
		/// \return Return a list containing all the scene nodes which are children of this node and whose name matches the specified one.
		vector<reference_wrapper<SceneNode>> FindNodeByName(const wstring & name);

		/// \brief Find all the nodes matching all the specified tags.
		/// \param tag Tags to find.
		/// \return Return a list containing all the scene nodes which are children of this node whose tags matches all the specified ones.
		vector<reference_wrapper<SceneNode>> FindNodeByTag(std::initializer_list<wstring> tags);

		// Identity

		/// \brief Check whether this object and the specified one are actually the same objects.
		/// \param other The other node to test against.
		bool operator==(const SceneNode & other) const ;

		/// \brief Check whether this object and the specified one are not the same object.
		/// \param other The other node to test against.
		bool operator!=(const SceneNode & other) const;

		/// \brief Get the scene this node belongs to.
		/// \return Returns the scene this node belongs to.
		Scene & GetScene();

		/// \brief Get the scene this node belongs to.
		/// \return Returns the scene this node belongs to.
		const Scene & GetScene() const;

		/// \brief Get the scene object's name.

		/// \return Returns the scene object's name.
		const wstring & GetName() const;

		/// \brief Add a new tag to the scene object.
		/// \param tag The new tag to add.
		void AddTag(const wstring & tag);

		/// \brief Remove an existing tag.
		/// \param tag The tag to remove.
		void RemoveTag(const wstring & tag);

		/// \brief Check whether the object has a particular tag.
		/// \param tag The tag to match.
		/// \return Returns true if the tag is found, false otherwise.
		bool HasTag(const wstring & tag) const;

		/// \brief Check whether the object has all the specified tags.
		/// \param tags The tags to match.
		/// \return Returns true if all the tags are found, false otherwise.
		bool HasTags(std::initializer_list<wstring> tags) const;

		/// \brief Get the unique ID identifying this scene node.
		/// \return Returns an object which is guaranteed to be unique among other scene nodes.
		const Unique<SceneNode> & GetUniqueID() const;

	private:

		/// \brief Type of the tag set.
		using TagSet = set<wstring>;

		// Transformation and hierarchy

		void SetDirty(bool world_only = false);

		void UpdateLocalTransform();

		void UpdateWorldTransform();

		void FindNodeByName(vector<reference_wrapper<SceneNode>> & nodes, const wstring & name);

		void FindNodeByTag(vector<reference_wrapper<SceneNode>> & nodes, std::initializer_list<wstring> tags);

		// Components

		ComponentList components_;

		// Transformation and hierarchy

		SceneNode* parent_;

		ChildrenList children_;

		Translation3f position_;							

		Quaternionf rotation_;								

		AlignedScaling3f scale_;							

		Affine3f local_transform_;							

		Affine3f world_transform_;							

		bool local_dirty_;									

		bool world_dirty_;									
		
		// Identity
		//Scene & scene_;

		wstring name_;

		TagSet tags_;

		Unique<SceneNode> unique_;

	};

	/// \brief Represents an entire scene.
	class Scene{

	public:

		/// \brief Create a new scene.
		Scene();

		/// \brief No assignment operator.
		Scene & operator=(const Scene &) = delete;

		/// \brief Update the entire scene.
		/// \param time The current application time.
		void Update(const Time & time);

		/// \brief Get the scene root.
		/// \return Return a reference to the scene root.
		SceneNode & GetRoot();

	private:

		SceneNode root_;				// Root of the scene

	};

	// SceneNodeBuilder

	inline SceneNodeBuilder::SceneNodeBuilder(SceneNode & parent):
		parent_(&parent){}

	inline void SceneNodeBuilder::SetParent(SceneNode & parent){

		parent_ = &parent;

	}

	inline void SceneNodeBuilder::SetName(const wstring & name){

		name_ = name;

	}

	inline void SceneNodeBuilder::SetPosition(const Translation3f & position){

		position_ = position;

	}

	inline void SceneNodeBuilder::SetRotation(const Quaternionf & rotation){

		rotation_ = rotation;

	}

	inline void SceneNodeBuilder::SetScaling(const AlignedScaling3f & scaling){

		scaling_ = scaling;

	}

	inline void SceneNodeBuilder::SetTags(initializer_list<wstring> tags){

		tags_ = tags;

	}

	inline SceneNode & SceneNodeBuilder::Build() const{

		// TODO: add a memory pool for placement new to speed up the creation of the nodes.

		auto & node = *new SceneNode(name_, position_, rotation_, scaling_, tags_);

		node.SetParent(*parent_);
		
		return node;

	}

	// SceneNode - Components

	template<typename TNodeComponent, typename... TArgs>
	std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, shared_ptr<TNodeComponent>> SceneNode::AddComponent(TArgs&&... args){

		auto component = std::make_shared<TNodeComponent>(*this, std::forward<TArgs>(args)...);

		components_.push_back(static_pointer_cast<NodeComponent>(component));

		return component;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	void SceneNode::RemoveComponents(){

		components_.erase(std::remove_if(components_.begin(),
			components_.end(),
			[](const shared_ptr<NodeComponent> & component){ return dynamic_pointer_cast<TNodeComponent>(component) != nullptr; }),
			components_.end());

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	void SceneNode::RemoveComponent(typename shared_ptr<TNodeComponent> component){

		auto node_component = static_pointer_cast<NodeComponent>(component);

		components_.erase(std::remove(components_.begin(),
			components_.end(),
			component),
			components_.end());

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename shared_ptr<TNodeComponent> SceneNode::GetComponent(){

		auto it = std::find_if(components_.begin(),
			components_.end(),
			[](const shared_ptr<NodeComponent> & component){ return dynamic_pointer_cast<TNodeComponent>(component) != nullptr; });

		return it != components_.end() ?
			static_pointer_cast<TNodeComponent>(*it) :
			nullptr;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename shared_ptr<const TNodeComponent> SceneNode::GetComponent() const{
		
		auto it = std::find_if(components_.begin(),
			components_.end(),
			[](const shared_ptr<NodeComponent> & component){ return dynamic_pointer_cast<TNodeComponent>(component) != nullptr; });

		return it != components_.end() ?
			static_pointer_cast<const TNodeComponent>(*it) :
			nullptr;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename vector < shared_ptr<TNodeComponent> > SceneNode::GetComponents(){

		vector<shared_ptr<TNodeComponent>> components;

		shared_ptr<TNodeComponent> component;

		for (auto it : components_){

			component = dynamic_pointer_cast<TNodeComponent>(it);

			if (component != nullptr){

				components.push_back(component);

			}

		}

		return components;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename vector < shared_ptr<const TNodeComponent> > SceneNode::GetComponents() const{

		vector<shared_ptr<const TNodeComponent>> components;

		shared_ptr<const TNodeComponent> component;

		for (auto it : components_){

			component = dynamic_pointer_cast<const TNodeComponent>(it);

			if (component != nullptr){

				components.push_back(component);

			}

		}

		return components;

	}
	
	// SceneNode - Transform & Hierarchy

	inline const Translation3f & SceneNode::GetPosition() const{

		return position_;

	}

	inline void SceneNode::SetPosition(const Translation3f & position){

		position_ = position;

		SetDirty();

	}

	inline const Quaternionf & SceneNode::GetRotation() const{

		return rotation_;

	}

	inline void SceneNode::SetRotation(const Quaternionf & rotation){

		rotation_ = rotation.normalized();

		SetDirty();

	}

	inline const AlignedScaling3f & SceneNode::GetScaling() const{

		return scale_;

	}

	inline void SceneNode::SetScaling(const AlignedScaling3f & scaling){

		scale_ = scaling;

		SetDirty();

	}

	inline const Affine3f & SceneNode::GetLocalTransform(){

		UpdateLocalTransform();	//Update by need

		return local_transform_;

	}

	inline const Affine3f & SceneNode::GetWorldTransform(){

		UpdateWorldTransform(); //Update by need

		return world_transform_;

	}

	inline SceneNode & SceneNode::GetParent(){

		if (!IsRoot()){

			return *parent_;

		}
		else{

			return *this;

		}
		
	}

	inline const SceneNode & SceneNode::GetParent() const {

		if (!IsRoot()){

			return *parent_;

		}
		else{

			return *this;

		}

	}
	
	inline bool SceneNode::IsRoot() const{

		return parent_ == nullptr;

	}

	inline SceneNode::ChildrenList::iterator SceneNode::ChildrenBegin(){

		return children_.begin();

	}

	inline SceneNode::ChildrenList::iterator SceneNode::ChildrenEnd(){

		return children_.end();

	}

	inline SceneNode::ChildrenList::const_iterator SceneNode::ChildrenBegin() const{

		return children_.cbegin();

	}

	inline SceneNode::ChildrenList::const_iterator SceneNode::ChildrenEnd() const{

		return children_.cend();

	}

	inline unsigned int SceneNode::GetChildrenCount() const{

		return static_cast<unsigned int>(children_.size());

	}

	inline SceneNodeBuilder SceneNode::GetBuilder(){

		return SceneNodeBuilder(*this);

	}

	// SceneNode - Identity

	inline bool SceneNode::operator==(const SceneNode & other) const{

		return unique_ == other.unique_;

	}

	inline bool SceneNode::operator!=(const SceneNode & other) const{

		return unique_ != other.unique_;

	}

	inline const wstring & SceneNode::GetName() const{

		return name_;

	}

	inline void SceneNode::AddTag(const wstring & tag){

		tags_.insert(tag);

	}

	inline void SceneNode::RemoveTag(const wstring & tag){

		tags_.erase(tag);

	}

	inline bool SceneNode::HasTag(const wstring & tag) const{

		return tags_.find(tag) != tags_.end();

	}

	inline bool SceneNode::HasTags(std::initializer_list<wstring> tags) const{

		return std::accumulate(tags.begin(), 
							   tags.end(), 
							   true, 
							   [this](bool value, const wstring & tag){
			
									return value && HasTag(tag); 
		
							   });

	}

	inline const Unique<SceneNode> & SceneNode::GetUniqueID() const{

		return unique_;

	}

	// Scene

	inline SceneNode & Scene::GetRoot(){

		return root_;

	}

}