/// \file scene.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <typeindex>
#include <typeinfo>
#include <string>
#include <map>

#include "component.h"
#include "timer.h"
#include "gimath.h"
#include "range.h"
#include "unique.h"
#include "observable.h"

using ::std::vector;
using ::std::wstring;
using ::std::type_index;
using ::std::map;
using ::Eigen::Affine3f;
using ::Eigen::Translation3f;
using ::Eigen::AlignedScaling3f;
using ::Eigen::Quaternionf;

namespace gi_lib{

	class Scene;
	class SceneNode;
	class Transform;

	/// \brief Represents a scene and all its content.
	/// \author Raffaele D. Facendola
	class Scene{

	public:

		/// \brief Default constructor.
		Scene();

		/// \brief No copy constructor.
		Scene(const Scene&) = delete;

		/// \brief Create a new scene node.
		/// \param name The name of the scene node.
		/// \return Returns the created scene node.
		SceneNode* CreateNode(const wstring& name);

		/// \brief Create a new scene node with Transform interface.
		/// \param name The name of the scene node.
		/// \param translation Local translation.
		/// \param rotation Local rotation.
		/// \param scaling Local scaling.
		/// \return Returns the created scene node.
		SceneNode* CreateNode(const wstring& name, const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scaling);

		
	private:



	};

	/// \brief Base component for all scene nodes.
	/// Exposes properties to identify the node and the scene it belongs to.
	/// \author Raffaele D. Facendola
	class SceneNode : public Component{

		friend class Scene;

	public:

		/// \brief No copy constructor.
		SceneNode(const SceneNode&) = delete;

		/// \brief Create a new scene node.
		/// \param scene The scene this node is associated to.
		/// \param name The node name.
		SceneNode(Scene& scene, const wstring& name);

		/// \brief Destructor.
		virtual ~SceneNode();

		/// \brief Get the scene this node is associated to.
		/// \return Returns the scene this node is associated to.
		Scene& GetScene();

		/// \brief Get the scene this node is associated to.
		/// \return Returns the scene this node is associated to.
		const Scene& GetScene() const;

		/// \brief Get the node name.
		/// The name may not be univocal.
		/// \return Returns the node name.
		const wstring& GetName() const;

		/// \brief Get the node unique identifier.
		/// \return Returns the node unique identifier.
		const Unique<SceneNode> GetUid() const;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;
		
	private:

		Scene& scene_;						///< \brief Scene owning this node.

		const wstring name_;				///< \brief Name of the node.

		const Unique<SceneNode> uid_;		///< \brief Unique id of the node.

	};

	/// \brief Expose 3D-space transform capabilities.
	/// The composite tranformation is calculated by applying the scaling first, the rotation second and the translation last.
	/// \author Raffaele D. Facendola
	class Transform : public Component{

	public:

		/// \brief Arguments for the OnTransformChanged event.
		struct OnTransformChangedEventArgs{

			Transform* transform;	///< \brief Transform node who triggered the event.

		};

		using range = Range < vector<Transform*>::iterator >;

		using const_range = Range < vector<Transform*>::const_iterator >;

		/// \brief Create a new transform interface.
		/// The local transform is initialized as an identity matrix.
		Transform();

		/// \brief Create a new transform interface.
		/// \param translation Local translation.
		/// \param rotation Local rotation.
		/// \param scale Local scale.
		Transform(const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scale);

		/// \brief Get the translation.
		/// \return Returns the translation component.
		const Translation3f & GetTranslation() const;

		/// \brief Set the translation.
		/// \param translation The new translation.
		void SetTranslation(const Translation3f & translation);

		/// \brief Get the rotation component.
		/// \return Returns the rotation component.
		const Quaternionf & GetRotation() const;

		/// \brief Set the rotation component.
		/// \param rotation The new rotation.
		void SetRotation(const Quaternionf & rotation);

		/// \brief Get the scaling component.
		/// \return Returns the scaling component.
		const AlignedScaling3f & GetScale() const;

		/// \brief Set the scaling component.
		/// \param scale The new scaling.
		void SetScale(const AlignedScaling3f & scale);

		/// \brief Get the local transfom.
		/// \return Returns the local transform matrix.
		const Affine3f & GetLocalTransform() const;

		/// \brief Get the global transfom.
		/// \return Returns the global transform matrix.
		const Affine3f & GetWorldTransform() const;

		/// \brief Get the parent transform.
		/// \return Returns the parent transform. If this transform is a root, returns nullptr instead.
		Transform* GetParent();

		/// \brief Get the parent transform.
		/// \return Returns the parent transform. If this transform is a root, returns nullptr instead.
		const Transform* GetParent() const;

		/// \brief Set the parent transform.
		/// \param parent The new parent transform. Set to nullptr to make this instance a root.
		void SetParent(Transform* parent);

		/// \brief Get the transform's children.
		/// \return Returns an iterable range over the transform's children.
		range GetChildren();

		/// \brief Get the transform's children.
		/// \return Returns an iterable range over the transform's children.
		const_range GetChildren() const;

		virtual TypeSet GetTypes() const override;

		/// \brief Event triggered when either the local or the composite transform matrix has been changed.
		Observable<OnTransformChangedEventArgs>& OnTransformChanged();

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		/// \brief Signals that the local or the world transform needs to be calculated again.
		/// This method will dirty every child node.
		/// \param world_only Set this to true to dirten the world matrix only, set this to false to dirten both the local and the world matrix.
		void SetDirty(bool world_only);	

		Transform* parent_;						///< \brief Parent transform.

		vector<Transform*> children_;			///< \brief Children transforms.

		Translation3f translation_;				///< \brief Translation component.

		Quaternionf rotation_;					///< \brief Rotation component.

		AlignedScaling3f scale_;				///< \brief Scale component.

		mutable Affine3f local_transform_;		///< \brief Local transform.

		mutable Affine3f world_transform_;		///< \brief Composite transform.

		mutable bool local_dirty_;				///< \brief The local transform needs to be recalculated.

		mutable bool world_dirty_;				///< \brief The world transform needs to be calculated.

		Event< OnTransformChangedEventArgs > on_transform_changed_;		///< \brief Triggered when the transform matrix has been changed.

	};


}