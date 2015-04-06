/// \file scene.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <map>
#include <unordered_map>
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
#include "bvh.h"
#include "timer.h"
#include "maybe.h"
#include "exceptions.h"
#include "unique.h"
#include "macros.h"

using std::wstring;
using std::multimap;
using std::unordered_multimap;
using std::unique_ptr;
using std::set;
using std::initializer_list;
using std::reference_wrapper;
using std::iterator;
using std::pair;

namespace gi_lib{
	
	class __Scene;
	class __SceneNode;
	class __SceneComponent;
	
	/// \brief A scene node.
	/// A node may represent a camera, a light, geometry and so on.
	/// SceneComponents may be added to customize the node behaviour.
	/// \author Raffaele D. Facendola.
	class __SceneNode{
		
		using ComponentMapType = unordered_multimap < type_index, shared_ptr<__SceneComponent > > ;
		
	public:

		/// \brief Iterator to a scene component.
		template <typename TComponent>
		class ComponentIterator {

		public:

			/// \brief Wraps a multimap iterator from the component map.
			ComponentIterator(ComponentMapType::iterator iterator);

			/// \brief Test for equality.
			bool operator==(const ComponentIterator<TComponent>& other);

			/// \brief Test for inequality.
			bool operator!=(const ComponentIterator<TComponent>& other);

			/// \brief Dereferencing operator.
			TComponent& operator*();

			/// \brief Prefix increment.
			ComponentIterator<TComponent>& operator++();

			/// \brief Postfix increment.
			ComponentIterator<TComponent> operator++(int);

		private:

			ComponentMapType::iterator iterator_;

		};

		/// \brief Pair of iterators defining an iterable range.
		template <typename TComponent>
		class ComponentRange{

		public:

			/// \brief Standard STL defines.
			typedef ComponentIterator<TComponent> iterator;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;
			typedef TComponent value_type;
			typedef TComponent* pointer;
			typedef TComponent& reference;

			/// \brief Create a new component range.
			/// \param range Range.
			ComponentRange(pair<ComponentMapType::iterator, ComponentMapType::iterator> range);

			/// \brief Create a new component range.
			/// \param begin Begin of the range.
			/// \param end End of the range.
			ComponentRange(ComponentMapType::iterator begin, ComponentMapType::iterator end);

			/// \brief Begin of the range.
			/// \return Returns an iterator pointing to the begin of the range.
			ComponentIterator<TComponent> begin();

			/// \brief End of the range.
			/// \return Returns an iterator pointing past the end of the range.
			ComponentIterator<TComponent> end();

			/// \brief Dereference the head of the range.
			/// Undefined behaviour if the range is empty.
			TComponent& operator*();

			/// \brief Is this range empty?
			/// \return Returns true if the range is empty, false otherwise.
			bool IsEmpty();

		private:

			ComponentMapType::iterator begin_;

			ComponentMapType::iterator end_;
			
		};

		/// \brief Add a new component to the scene node.
		/// \tparam TComponent Type of the component to add. Must derive from SceneComponent.
		/// \tparam TArgs Type of the arguments that will be passed to the component's constructor.
		/// \param arguments Arguments that will be passed to the component's constructor.
		/// \return Returns an unary range containing the new component.
		template < typename TComponent, typename... TArgs >
		DERIVES_FROM_T(TComponent, __SceneComponent, ComponentRange<TComponent>) AddComponent(TArgs&&... arguments);

		/// \brief Remove a component from the node.
		/// \param component Component to remove. 
		void RemoveComponent(__SceneComponent& component);

		/// \brief Removes the components of the specified type.
		/// \tparam TComponent Type of the components to remove.
		template < typename TComponent >
		void RemoveComponents();

		/// \brief Get the first component matching the specified type.
		/// \tparam TComponent type of the component to get.
		/// \return Returns an unary range containing the first component matching the specified type if any. Returns an empty range if no component could be found.
		template < typename TComponent >
		ComponentRange<TComponent> GetComponent();

		/// \brief Get the first component matching the specified type.
		/// \tparam TComponent type of the component to get.
		/// \return Returns an unary range containing the first component matching the specified type if any. Returns an empty range if no component could be found.
		template < typename TComponent >
		ComponentRange<const TComponent> GetComponent() const;

		/// \brief Get the collection of all the components matching the specified type.
		///\ tparam TComponent type of the components to get.
		/// \return Returns an iterable range containing all the components matching the specified type. If no component was found, the range is empty.
		template < typename TComponent >
		ComponentRange<TComponent> GetComponents();

		/// \brief Get the collection of all the components matching the specified type.
		///\ tparam TComponent type of the components to get.
		/// \return Returns an iterable range containing all the components matching the specified type. If no component was found, the range is empty.
		template < typename TComponent >
		ComponentRange<const TComponent> GetComponents() const;

	private:

		pair<ComponentMapType::iterator, ComponentMapType::iterator> AddComponent(shared_ptr<__SceneComponent> component);

		void RemoveComponents(type_index component_type);

		wstring name_;								///< \brief Name of the node.

		ComponentMapType component_map_;			///< \brief Multimap used to associated types to each component.

	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ __SCENE NODE :: COMPONENT ITERATOR ~~~~~~~~~~~~~~~~~~~~~~~~~~~//

	template <typename TComponent>
	inline  __SceneNode::ComponentIterator<TComponent>::ComponentIterator(ComponentMapType::iterator iterator){

		iterator_ = iterator;

	}

	template <typename TComponent>
	inline bool __SceneNode::ComponentIterator<TComponent>::operator==(const ComponentIterator<TComponent>& other){

		return iterator_ == other.iterator_;

	}

	template <typename TComponent>
	inline bool __SceneNode::ComponentIterator<TComponent>::operator!=(const ComponentIterator<TComponent>& other){

		return iterator_ != other.iterator_;

	}

	template <typename TComponent>
	inline TComponent& __SceneNode::ComponentIterator<TComponent>::operator*(){

		return *static_cast<TComponent*>(iterator_->second);

	}

	template <typename TComponent>
	inline __SceneNode::ComponentIterator<TComponent>& __SceneNode::ComponentIterator<TComponent>::operator++(){

		return ++iterator;

	}

	template <typename TComponent>
	inline __SceneNode::ComponentIterator<TComponent> __SceneNode::ComponentIterator<TComponent>::operator++(int){

		return iterator++;

	}

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ __SCENE NODE :: COMPONENT RANGE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

	template <typename TComponent>
	inline __SceneNode::ComponentRange<TComponent>::ComponentRange(pair<ComponentMapType::iterator, ComponentMapType::iterator> range) :
		ComponentRange(range.first, range.second){}

	template <typename TComponent>
	inline __SceneNode::ComponentRange<TComponent>::ComponentRange(ComponentMapType::iterator begin, ComponentMapType::iterator end){

		begin_ = begin;
		end_ = end;

	}

	template <typename TComponent>
	inline __SceneNode::ComponentIterator<TComponent> __SceneNode::ComponentRange<TComponent>::begin(){

		return ComponentIterator<TComponent>(begin_);
		
	}

	template <typename TComponent>
	inline __SceneNode::ComponentIterator<TComponent> __SceneNode::ComponentRange<TComponent>::end(){

		return ComponentIterator<TComponent>(end_);

	}

	template <typename TComponent>
	TComponent& __SceneNode::ComponentRange<TComponent>::operator*(){

		return *static_cast<TComponent*>(begin_->second);

	}

	template <typename TComponent>
	inline bool __SceneNode::ComponentRange<TComponent>::IsEmpty(){

		return begin_ == end_;

	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ __SCENE NODE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

	template <typename TComponent, typename... TArgs >
	DERIVES_FROM_T(TComponent, __SceneComponent, __SceneNode::ComponentRange<TComponent>) __SceneNode::AddComponent(TArgs&&... arguments){

		auto component = make_shared<TComponent>(forward(arguments)...);

		component->scene_node_ = this;

		return ComponentRange<TComponent>(AddComponent(static_pointer_cast<__SceneComponent>(component)))
		
	}

	template < typename TComponent >
	inline void __SceneNode::RemoveComponents(){

		RemoveComponents(type_index(typeid(TComponent)));

	}

	template < typename TComponent >
	__SceneNode::ComponentRange<TComponent> __SceneNode::GetComponent(){

		auto it = component_type_map_.find(type_index(typeid(TComponent)));

		auto next = it;

		if (it != component_type_map_.end()){

			++next;

		}

		return ComponentRange<TComponent>(it, next);

	}
	
	template < typename TComponent >
	__SceneNode::ComponentRange<const TComponent> __SceneNode::GetComponent() const{

		auto it = component_type_map_.find(type_index(typeid(TComponent)));

		auto next = it;

		if (it != component_type_map_.end()){

			++next;

		}

		return ComponentRange<const TComponent>(it, next);

	}

	template < typename TComponent >
	__SceneNode::ComponentRange<TComponent> __SceneNode::GetComponents(){

		auto range = component_type_map_.equal_range(type_index(typeid(TComponent)));

		return ComponentRange<TComponent>(range.first,
										  range.second);

	}

	template < typename TComponent >
	__SceneNode::ComponentRange<const TComponent> __SceneNode::GetComponents() const{

		auto range = component_type_map_.equal_range(type_index(typeid(TComponent)));

		return ComponentRange<const TComponent>(range.first,
												range.second);

	}

	/// \brief Component of a scene node.
	/// \author Raffaele D. Facendola.
	class __SceneComponent{

		friend SceneNode;

	public:

		__SceneComponent();

	protected:

		template < typename TType>
		bool InstanceOf();

	private:

		__SceneNode* scene_node_;


	};
	


	class Scene;
	class SceneNode;
	
	/// \brief Represents a scene node.

	/// A node may represents a camera, a light, a model and so on.
	/// NodeComponents may be plugged to customize its behaviour
	/// \author Raffaele D. Facendola
	class SceneNode{

		friend class Scene;

	public:
		
		/// \brief Type of the component list.
		using ComponentList = vector < unique_ptr<NodeComponent> >;

		/// \brief Type of the children list.
		using ChildrenList = vector < unique_ptr<SceneNode> > ;

		/// \brief Create a scene node.
		/// \param scene The scene this node belongs to.
		/// \param name Name of the scene node. It may not be unique.
		/// \param position Position of the node in local space.
		/// \param rotation Rotation of the node in local space.
		/// \param scaling Scaling of the node in local space.
		/// \param tags List of tags associated to the scene node.
		SceneNode(Scene & scene, const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags);

		/// \brief Destroy the current scene node and all the children.
		virtual ~SceneNode();
		
		/// \brief No copy constructor.
		SceneNode(const SceneNode &) = delete;

		/// \brief No assignment operator.
		SceneNode & operator=(const SceneNode &) = delete;
		
		/// \brief Add a new component.
		/// \tparam TNodeComponent Type of the component to add. It must derive from NodeComponent.
		/// \param args Arguments to pass to the component's constructor.
		template<typename TNodeComponent, typename... TArgs>
		std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, TNodeComponent*> AddComponent(TArgs&&... args);

		/// \brief Remove all the component whose dynamic type matches the specified one.

		/// \tparam Dynamic type of the components to remove. It must derive from NodeComponent.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		void RemoveComponents();

		/// \brief Remove the specified component.
		/// \tparam TNodeComponent Type of the component to remove. It must derive from NodeComponent.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		void RemoveComponent(TNodeComponent * component);

		/// \brief Get the first component whose dynamic type matched the specified one.
		/// \tparam TNodeComponent Dynamic type of the component to get. It must derive from NodeComponent.
		/// \return Returns a pointer to the first component matching the specified type. Returns null if no component could be found.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename TNodeComponent * GetComponent();

		/// \brief Get the first component whose dynamic type matched the specified one.
		/// \tparam TNodeComponent Dynamic type of the component to get. It must derive from NodeComponent.
		/// \return Returns a pointer to the first component matching the specified type. Returns null if no component could be found.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename const TNodeComponent * GetComponent() const;

		/// \brief Get the list of all components whose dynamic type match the specified one.
		/// \tparam TNodeComponent Dynamic type of the components to get. It must derive from NodeComponent.
		/// \return Returns the list of all components matching the specified type.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename vector < TNodeComponent * > GetComponents();

		/// \brief Get the list of all components whose dynamic type match the specified one.
		/// \tparam TNodeComponent Dynamic type of the components to get. It must derive from NodeComponent.
		/// \return Returns the list of all components matching the specified type.
		template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type* = nullptr>
		typename vector < const TNodeComponent * > GetComponents() const;

		/// \brief Prepare the node for the update.

		/// This method is used to update nodes before their components.
		/// \param time The application time.
		void PreUpdate(const Time & time);

		/// \brief Updates the enabled components.

		/// \param time The application time.
		void Update(const Time & time);

		/// \brief Post updats the node.

		/// The method is intended to update the internal state of the components based on the state of others.
		/// Avoid cross-component update here!
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
		const Affine3f & GetLocalTransform() const;

		/// \brief Get the global transfom.
		/// \return Returns the global transform matrix.
		const Affine3f & GetWorldTransform() const;

		/// \brief Check whether the world transform changed since the last update.
		/// \return Returns true if the world transform changed since the last update, false otherwise.
		bool IsWorldTransformChanged() const;

		/// \brief Get this node's parent.
		/// \return Returns a pointer to the parent node if any. Returns nullptr otherwise.
		SceneNode * GetParent();
		
		/// \brief Get this node's parent.
		/// \return Returns a pointer to the parent node if any. Returns nullptr otherwise.
		const SceneNode * GetParent() const;

		/// \brief Set this node's parent.

		/// The specified parent will take node's ownership and will destroy this node upon its own destruction.
		/// \param parent The new parent.
		void SetParent(SceneNode & parent);

		/// \brief Get the children count.
		/// \return Returns the children count.
		unsigned int GetChildrenCount() const;

		/// \brief Get a children knowing its index.

		/// The method throws if the index exceeds the number of the actual children.
		/// \return Return the child whose index is the specified one.
		SceneNode & GetChildAt(unsigned int index);

		/// \brief Get a children knowing its index.

		/// The method throws if the index exceeds the number of the actual children.
		/// \return Return the child whose index is the specified one.
		const SceneNode & GetChildAt(unsigned int index) const;

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

		void SetDirty(bool world_only = false) const;

		void UpdateLocalTransform() const;

		void UpdateWorldTransform() const;

		SceneNode & AddNode(unique_ptr<SceneNode> && node);

		unique_ptr<SceneNode> MoveNode(SceneNode & node);

		void DestroyNode(SceneNode & node);
		
		// Identity

		void FindNodeByName(const wstring & name, vector<SceneNode *> & nodes);

		void FindNodeByTag(std::initializer_list<wstring> & tags, vector<SceneNode *> nodes);
		
		// Components

		ComponentList components_;

		// Transformation and hierarchy

		SceneNode* parent_;

		ChildrenList children_;

		Translation3f position_;							

		Quaternionf rotation_;								

		AlignedScaling3f scale_;							

		mutable Affine3f local_transform_;							

		mutable Affine3f world_transform_;							

		mutable bool world_changed_;

		mutable bool local_dirty_;									

		mutable bool world_dirty_;									
		
		// Identity
		Scene & scene_;

		wstring name_;

		TagSet tags_;

		Unique<SceneNode> unique_;

	};

	/// \brief Represents an entire scene.
	class Scene{

		friend class Camera;

	public:

		/// \brief Create a new scene.
		Scene();
				
		/// \brief Destroy the scene instance.
		~Scene();

		/// \brief No copy ctor
		Scene(const Scene &) = delete;

		/// \brief No assignment operator.
		Scene & operator=(const Scene &) = delete;

		/// \brief Create a new node.
		/// \param name The name of the node to add.
		/// \param position The position of the node with respect to the world's origin.
		/// \param rotation The rotation of the node.
		/// \param scaling The scaling of the node.
		/// \param tags The tags associated to the node to add.
		/// \return Returns the created node.
		SceneNode & CreateNode(const wstring & name, const Translation3f & position, const Quaternionf & rotation, const AlignedScaling3f & scaling, initializer_list<wstring> tags);

		/// \brief Create a new node.
		/// \return Returns the created node.
		SceneNode & CreateNode();

		/// \brief Destroy an existing node.
		void DestroyNode(SceneNode & node);

		/// \brief Get the scene root.
		SceneNode & GetRoot();

		/// \brief Find all the nodes matching the specified name.
		/// \param name The name to find.
		/// \return Return a list containing all the nodes whose name matches the specified one.
		vector<SceneNode *> FindNodeByName(const wstring & name);

		/// \brief Find all the nodes matching all the specified tags.
		/// \param tags Tags to find.
		/// \return Return a list containing all the nodes whose tags match all the specified ones.
		vector<SceneNode *> FindNodeByTag(std::initializer_list<wstring> tags);

		/// \brief Update the entire scene.
		/// \param time The current application time.
		void Update(const Time & time);

		/// \brief Get the scene's bounding volume hierarchy.
		/// \return Returns a reference to the scene's bounding volume hierarchy.
		BVH & GetBVH();

		/// \brief Get the scene's bounding volume hierarchy.
		/// \return Returns a reference to the scene's bounding volume hierarchy.
		const BVH & GetBVH() const;

		/// \brief Get the scene's cameras.
		/// \return Returns a vector containing all the cameras inside the scene.
		vector<Camera *> GetCameras();

		/// \brief Get the scene's cameras.
		/// \return Returns a vector containing all the cameras inside the scene.
		const vector<Camera *> & GetCameras() const;

	private:

		void AddCamera(Camera & camera);

		void RemoveCamera(Camera & camera);

		void SortCamerasByPriority();
		
		unique_ptr<SceneNode> root_;

		unique_ptr<BVH> bvh_;			// Bounding volume hierarchy

		vector<Camera *> cameras_;

	};

	// SceneNode - Components

	template<typename TNodeComponent, typename... TArgs>
	std::enable_if_t<std::is_base_of<NodeComponent, TNodeComponent>::value, TNodeComponent*> SceneNode::AddComponent(TArgs&&... args){

		components_.push_back(make_unique<TNodeComponent>(*this, std::forward<TArgs>(args)...));

		return static_cast<TNodeComponent*>(components_.back().get());

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	void SceneNode::RemoveComponents(){

		components_.erase(std::remove_if(components_.begin(),
			components_.end(),
			[](const unique_ptr<NodeComponent> & component){ return dynamic_cast<TNodeComponent *>(component.get()) != nullptr; }),
			components_.end());

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	void SceneNode::RemoveComponent(typename TNodeComponent * component){

		components_.erase(std::remove_if(components_.begin(),
			components_.end(),
			[](const unique_ptr<NodeComponent> & component){ return component.get() == static_cast<NodeComponent *>(component); }),
			components_.end());

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename TNodeComponent * SceneNode::GetComponent(){

		auto it = std::find_if(components_.begin(),
			components_.end(),
			[](const unique_ptr<NodeComponent> & component){ return dynamic_cast<TNodeComponent *>(component.get()) != nullptr; });

		return it != components_.end() ?
			static_cast<TNodeComponent *>(it->get()) :
			nullptr;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename const TNodeComponent *SceneNode::GetComponent() const{
		
		auto it = std::find_if(components_.begin(),
			components_.end(),
			[](const unique_ptr<NodeComponent> & component){ return dynamic_cast<TNodeComponent *>(component.get()) != nullptr; });

		return it != components_.end() ?
			static_cast<const TNodeComponent *>(it->get()) :
			nullptr;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename vector < TNodeComponent * > SceneNode::GetComponents(){

		vector< TNodeComponent *> components;

		TNodeComponent * component;

		for (auto & it : components_){

			component = dynamic_cast<TNodeComponent * >(it.get());

			if (component != nullptr){

				components.push_back(component);

			}

		}

		return components;

	}

	template<typename TNodeComponent, typename std::enable_if<std::is_base_of<NodeComponent, TNodeComponent>::value>::type*>
	typename vector < const TNodeComponent * > SceneNode::GetComponents() const{

		vector< const TNodeComponent *> components;

		const TNodeComponent * component;

		for (auto & it : components_){

			component = dynamic_cast<const TNodeComponent * >(it.get());

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

	inline const Affine3f & SceneNode::GetLocalTransform() const{

		UpdateLocalTransform();	//Update by need

		return local_transform_;

	}

	inline const Affine3f & SceneNode::GetWorldTransform() const{

		UpdateWorldTransform(); //Update by need

		return world_transform_;

	}

	inline bool SceneNode::IsWorldTransformChanged() const{

		return world_changed_;

	}

	inline SceneNode * SceneNode::GetParent(){

		return parent_;
		
	}

	inline const SceneNode * SceneNode::GetParent() const {
		
		return parent_;

	}
	
	inline unsigned int SceneNode::GetChildrenCount() const{

		return static_cast<unsigned int>(children_.size());

	}

	inline SceneNode & SceneNode::GetChildAt(unsigned int index){

		return *children_[index];

	}

	inline const SceneNode & SceneNode::GetChildAt(unsigned int index) const{

		return *children_[index];

	}

	// SceneNode - Identity

	inline bool SceneNode::operator==(const SceneNode & other) const{

		return unique_ == other.unique_;

	}

	inline bool SceneNode::operator!=(const SceneNode & other) const{

		return unique_ != other.unique_;

	}

	inline Scene & SceneNode::GetScene(){

		return scene_;

	}

	inline const Scene & SceneNode::GetScene() const{

		return scene_;

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

		return *root_;

	}

	inline BVH & Scene::GetBVH(){

		return *bvh_;

	}

	inline const BVH & Scene::GetBVH() const{

		return *bvh_;

	}

	inline vector<Camera *> Scene::GetCameras(){

		return cameras_;

	}

	inline const vector<Camera *> & Scene::GetCameras() const{

		return cameras_;

	}

}