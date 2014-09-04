/// \file components.h
/// \brief Defines the base classes used to manage the scene node components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <Eigen\Geometry>
#include <vector>

#include "timer.h"

using Eigen::Affine3f;
using std::vector;

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

	public:

		/// \brief Initialize the transform component with an identity matrix.
		Transform();

		/// \brief Initialize the transform component with a transformation matrix.
		Transform(const Affine3f & local_transform);

		/// \brief Copy-constructor.

		/// The constructor copy the matrix value and the relationships with other transforms.
		Transform(const Transform & other);

		/// \brief Move-constructor.

		/// The constructor moves the matrix value and the relationship with other transforms.
		Transform(Transform && other);

		/// \brief Add a new child to this transform.

		/// If the child has a different parent, the relationship is deleted.
		/// \param child The child transformation.
		/// \return Returns a reference to this instance.
		Transform & AddChild(const Transform & child);

		/// \brief Remove a child transformation.
		
		/// The child's parent becomes null.
		/// \param child The child to remove.
		/// \return Returns a reference to this instance.
		Transform & RemoveChild(const Transform & child);

		/// \brief Get the parent transform.

		/// \return Returns the transform's parent if present. Returns nullptr if this component has no parent transform.
		Transform * GetParent();

		/// \brief Get the parent transform.

		/// \return Returns the transform's parent if present. Returns nullptr if this component has no parent transform.
		const Transform * GetParent() const;

		/// \brief Get an iterator to the transform's childs.
		/// \return Returns an iterator pointing to the beginning of the transform's child collection.
		vector<Transform *>::iterator GetChildBegin();

		/// \brief Get an iterator to the transform's childs.
		/// \return Returns an iterator pointing past the end of the transform's child collection.
		vector<Transform *>::iterator GetChildEnd();

		/// \brief Get an iterator to the transform's childs.
		/// \return Returns an iterator pointing to the beginning of the transform's child collection.
		vector<Transform *>::const_iterator GetChildBegin() const;

		/// \brief Get an iterator to the transform's childs.
		/// \return Returns an iterator pointing past the end of the transform's child collection.
		vector<Transform *>::const_iterator GetChildEnd() const;

		/// \brief Get the current number of child transforms attached to this component.
		/// \return Returns the number of the child transforms attached to this component.
		unsigned int GetChildCount() const;

	protected:

		/// \brief Update the components recursively
		virtual void Update(const Time & time);

	private:

		Transform * parent_;			//Parent of the transformation.

		vector<Transform *> childs_;	//Child of the transformation.

		Affine3f local_transform_;

		Affine3f world_transform_;		//Compount transformation.

	};

	//

	NodeComponent::NodeComponent() :
		owner_(nullptr),
		enabled_(true){}

	NodeComponent::~NodeComponent(){}

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

}
