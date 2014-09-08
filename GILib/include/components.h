/// \file components.h
/// \brief Defines the base classes used to manage the scene node components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "gimath.h"
#include "timer.h"
#include "maybe.h"

using ::Eigen::Affine3f;
using ::std::vector;

namespace gi_lib{

	/// \brief Scene object component.
	/// \author Raffaele D. Facendola
	class NodeComponent{

		friend class SceneNode;

	public:

		/// \brief Default constructor.
		NodeComponent();

		/// \brief No copy constructor.
		NodeComponent(const NodeComponent & other) = delete;

		/// \brief No assignment operator.
		NodeComponent & operator=(const NodeComponent & other) = delete;

		virtual ~NodeComponent();

		/// \brief Get the component's owner.

		/// \return Returns the component's owner reference.
		SceneNode & GetOwner();

		/// \brief Get the component's owner.

		/// \return Returns the component's owner constant reference.
		const SceneNode & GetOwner() const;

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

		SceneNode * owner_;

	};

	//

	/// \brief Represents a 3D transformation.
	class Transform: public NodeComponent{

		friend class SceneNode;

	public:

		/// \brief Initialize the transform component with a transformation matrix.
		/// \param local_transform Transformation in local space.
		Transform(const Affine3f & local_transform);

		/// \brief No copy constructor.
		/// A node may have one parent at maximum, copying would require for them to have more than one.
		Transform(const Transform & other) = delete;

		/// \brief No assignment operator.
		/// A node may have one parent at maximum, assignment would require for them to have more than one.
		Transform & operator=(const Transform & other) = delete;

		/// \brief Move-constructor.

		/// The constructor moves the matrix value and the relationship with other transforms.
		Transform(Transform && other);

		/// \brief Destroy the transfom.

		/// \brief Child transforms become roots.
		~Transform();

		/// \brief Get the local transfom.
		/// \return Returns the local transform matrix.
		Affine3f & GetLocalTransform();
		
		/// \brief Get the local transfom.
		/// \return Returns the local transform matrix.
		const Affine3f & GetLocalTransform() const;

		/// \brief Get the global transfom.
		/// \return Returns the global transform matrix.
		Affine3f & GetWorldTransform();
		
		/// \brief Get the global transfom.
		/// \return Returns the global transform matrix.
		const Affine3f & GetWorldTransform() const;

		/// \brief Assign this transform to a new parent.

		/// The transform is detached first and its previous parent is updated accordingly.
		void SetParent(Transform & parent);

		/// \brief Get the parent transform.

		/// \return Returns the transform's parent.
		Transform & GetParent();

		/// \brief Get the parent transform.

		/// \return Returns the transform's parent if present. Returns nullptr if this component has no parent transform.
		const Transform & GetParent() const;

		/// \brief Check whether this node is a root.
		/// \return Returns true if the node is a root, false otherwise.
		bool IsRoot() const;

		/// \brief Get a child transform by index.
		/// \param index Index of the child.
		/// \return Returns a reference to the child transform.
		Transform & GetChildAt(unsigned int index);

		/// \brief Get a child transform by index.
		/// \param index Index of the child.
		/// \return Returns a const reference to the child transform.
		const Transform & GetChildAt(unsigned int index) const;

		/// \brief Get the current number of child transforms attached to this component.
		/// \return Returns the number of the child transforms attached to this component.
		size_t GetChildCount() const;

	protected:

		virtual void Update(const Time & time) override;

	private:
		
		void UpdateOwner(const Time & time);

		void RemoveChild(Transform & child);

		void AddChild(Transform & child);

		Transform * parent_;			//Parent of the transformation.

		vector<Transform *> children_;	//Children of the transformation.

		Affine3f local_transform_;

		Affine3f world_transform_;		//Compount transformation.

	};

	//

	inline SceneNode & NodeComponent::GetOwner(){

		return *owner_;

	}

	inline const SceneNode & NodeComponent::GetOwner() const{

		return *owner_;

	}

	inline bool NodeComponent::IsEnabled() const{

		return enabled_;

	}

	inline void NodeComponent::SetEnabled(bool enabled){

		enabled_ = enabled;

	}

	//

	inline Affine3f & Transform::GetLocalTransform(){

		return local_transform_;

	}

	inline const Affine3f & Transform::GetLocalTransform() const{

		return local_transform_;

	}
	
	inline Affine3f & Transform::GetWorldTransform(){

		return world_transform_;

	}

	inline const Affine3f & Transform::GetWorldTransform() const{

		return world_transform_;

	}

	inline bool Transform::IsRoot() const{

		return parent_ == nullptr;

	}
	
	inline Transform & Transform::GetChildAt(unsigned int index){

		return *children_[index];

	}

	inline const Transform & Transform::GetChildAt(unsigned int index) const{

		return *children_[index];

	}

	inline size_t Transform::GetChildCount() const{

		return children_.size();

	}
	
}
