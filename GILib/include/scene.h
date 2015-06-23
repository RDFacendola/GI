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
#include <memory>

#include "component.h"
#include "timer.h"
#include "gimath.h"
#include "gilib.h"
#include "range.h"
#include "unique.h"
#include "observable.h"
#include "resources.h"
#include "graphics.h"
#include "volume_hierarchy.h"

using ::std::vector;
using ::std::wstring;
using ::std::unique_ptr;

namespace gi_lib{

	class Scene;
	class NodeComponent;
	class TransformComponent;
	class CameraComponent;
	
	class Mesh;

	/// \brief Represents a scene and all its content.
	/// \author Raffaele D. Facendola
	class Scene{

	public:

		/// \brief Default constructor.
		Scene(unique_ptr<IVolumeHierarchy> volume_hierarchy);

		/// \brief Scene destructor.
		~Scene();

		/// \brief No copy constructor.
		Scene(const Scene&) = delete;

		/// \brief Create a new empty node.
		/// \param name The name of the node.
		/// \return Returns a pointer to the created node.
		NodeComponent* CreateNode(const wstring& name);

		/// \brief Create a new scene node with a TransformComponent.
		/// \param name The name of the node.
		/// \param translation Node translation.
		/// \param rotation Node rotation.
		/// \param scale Node scale.
		/// \return Returns a pointer to the created node.
		TransformComponent* CreateNode(const wstring& name, const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scale);

		/// \brief Get the main camera of the scene.
		/// \return Returns the main camera of the scene.
		CameraComponent* GetMainCamera();

		/// \brief Get the main camera of the scene.
		/// \return Returns the main camera of the scene.
		const CameraComponent* GetMainCamera() const;

		/// \brief Set the main camera of the scene.
		/// \param The new main camera.
		/// \remarks The camera component must belong to the scene.
		void SetMainCamera(CameraComponent* camera);

		/// \brief Get the volume hierarchy.
		/// \return Returns the volume hierarchy.
		IVolumeHierarchy& GetVolumeHierarchy();

		/// \brief Get the volume hierarchy.
		/// \return Returns the volume hierarchy.
		const IVolumeHierarchy& GetVolumeHierarchy() const;

		/// \brief Get the list of the nodes created so far.
		/// \return Returns the list of the nodes created so far.
		const vector<NodeComponent*>& GetNodes() const;

		/// \brief Get the list of the nodes created so far.
		/// \return Returns the list of the nodes created so far.
		vector<NodeComponent*>& GetNodes();

	private:

		vector<NodeComponent*> nodes_;						///< \brief Nodes inside the scene.

		CameraComponent* main_camera_;						///< \brief Main camera.

		unique_ptr<IVolumeHierarchy> volume_hierarchy_;		///< \brief Scene volume hierarchy.

	};

	/// \brief Node component used to link a scene to its nodes.
	/// \author Raffaele D. Facendola
	class NodeComponent : public Component{

		friend class Scene;

	public:

		/// \brief No copy constructor.
		NodeComponent(const NodeComponent&) = delete;

		/// \brief Create a new scene node.
		/// \param scene The scene this node is associated to.
		/// \param name The node name.
		NodeComponent(Scene& scene, const wstring& name);

		/// \brief Destructor.
		virtual ~NodeComponent();

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
		const Unique<NodeComponent> GetUid() const;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;
		
	private:

		Scene& scene_;						///< \brief Scene owning this node.

		const wstring name_;				///< \brief Name of the node.

		const Unique<NodeComponent> uid_;	///< \brief Unique id of the node.

	};

	/// \brief Expose 3D-space transform capabilities.
	/// The composite transformation is calculated by applying the scaling first, the rotation second and the translation last.
	/// \author Raffaele D. Facendola
	class TransformComponent : public Component{

	public:

		/// \brief Arguments for the OnTransformChanged event.
		struct OnTransformChangedEventArgs{

			TransformComponent* transform;	///< \brief Transform node who triggered the event.

		};

		using range = Range < vector<TransformComponent*>::iterator >;

		using const_range = Range < vector<TransformComponent*>::const_iterator >;

		/// \brief Create a new transform interface.
		/// The local transform is initialized as an identity matrix.
		TransformComponent();

		/// \brief Create a new transform interface.
		/// \param translation Local translation.
		/// \param rotation Local rotation.
		/// \param scale Local scale.
		TransformComponent(const Translation3f& translation, const Quaternionf& rotation, const AlignedScaling3f& scale);

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

		/// \brief Get the right direction.
		/// The right direction defines the positive X-axis in local space.
		/// \return Returns the right direction.
		Vector3f GetRight() const;

		/// \brief Get the up direction.
		/// The up direction defines the positive Y-axis in local space.
		/// \return Returns the up direction.
		Vector3f GetUp() const;

		/// \brief Get the forward direction.
		/// The forward direction defines the positive Z-axis in local space.
		/// \return Returns the forward direction.
		Vector3f GetForward() const;

		/// \brief Get the local transform.
		/// \return Returns the local transform matrix.
		const Affine3f & GetLocalTransform() const;

		/// \brief Get the global transform.
		/// \return Returns the global transform matrix.
		const Affine3f & GetWorldTransform() const;

		/// \brief Get the parent transform.
		/// \return Returns the parent transform. If this transform is a root, returns nullptr instead.
		TransformComponent* GetParent();

		/// \brief Get the parent transform.
		/// \return Returns the parent transform. If this transform is a root, returns nullptr instead.
		const TransformComponent* GetParent() const;

		/// \brief Set the parent transform.
		/// \param parent The new parent transform. Set to nullptr to make this instance a root.
		void SetParent(TransformComponent* parent);

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

		TransformComponent* parent_;			///< \brief Parent transform.

		vector<TransformComponent*> children_;	///< \brief Children transforms.

		Translation3f translation_;				///< \brief Translation component.

		Quaternionf rotation_;					///< \brief Rotation component.

		AlignedScaling3f scale_;				///< \brief Scale component.

		mutable Affine3f local_transform_;		///< \brief Local transform.

		mutable Affine3f world_transform_;		///< \brief Composite transform.

		mutable bool local_dirty_;				///< \brief The local transform needs to be recalculated.

		mutable bool world_dirty_;				///< \brief The world transform needs to be calculated.

		Event< OnTransformChangedEventArgs > on_transform_changed_;		///< \brief Triggered when the transform matrix has been changed.

	};

	/// \brief Contains informations about an axis-aligned bounding box surrounding the node.
	/// \author Raffaele D. Facendola
	class VolumeComponent : public Component{

	public:

		/// \brief Arguments relative to OnBoundsChanged event.
		struct OnBoundsChangedEventArgs{

			VolumeComponent* volume;	///< \brief Volume whose bounds have changed.

		};

		/// \brief Create a new volume component.
		VolumeComponent();

		/// \brief Create a new volume component.
		/// \param bounds The initial bounds of the component.
		VolumeComponent(const AABB& bounds);

		/// \brief Virtual destructor.
		virtual ~VolumeComponent();

		/// \brief Get the transformed bounding box.
		/// \return Returns the transformed bounding box.
		const AABB& GetBoundingBox() const;

		/// \brief Get the transformed bounding sphere.
		/// \return Returns the transformed bounding sphere.
		const Sphere& GetBoundingSphere() const;

		/// \brief Event that is signaled whenever the bounds change.
		/// \return Returns the event that is signaled whenever the bounds change.
		Observable<OnBoundsChangedEventArgs>& OnBoundsChanged();

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

		/// \brief Set new bounds for this component.
		/// \param bounds New bounds.
		void SetBoundingBox(const AABB& bounds);

	private:

		void SetDirty();										///< \brief Mark this component as dirty and notify everybody.

		AABB bounding_box_;										///< \brief Bounding box.

		TransformComponent* transform_;							///< \brief Transform component needed to computed the transformed bounds.

		Event<OnBoundsChangedEventArgs> on_bounds_changed_;		///< \brief Event signaled whenever the bounds change.

		unique_ptr<Listener> on_transform_changed_lister_;		///< \brief Listener for the transform changed event.

		mutable AABB transformed_bounds_;						///< \brief Transformed bounds.

		mutable bool is_box_dirty_;								///< \brief Whether the bounds needs to be recalculated.

		mutable Sphere bounding_sphere_;						///< \brief Bounding sphere. Calculated by need.

		mutable bool is_sphere_dirty_;							///< \brief Is the bounding sphere dirty?

	};

	/// \brief Mesh component.
	/// \author Raffaele D. Facendola.
	class MeshComponent : public VolumeComponent{

	public:

		/// \brief Create an empty mesh component.
		MeshComponent();

		/// \brief Create a new mesh component.
		/// \param Static mesh associated to this component
		MeshComponent(ObjectPtr<Mesh> mesh);
		
		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		ObjectPtr<Mesh> GetMesh();

		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		ObjectPtr<const Mesh> GetMesh() const;

		/// \brief Set the mesh associated to this component.
		/// \param mesh The new mesh to associate.
		void SetMesh(ObjectPtr<Mesh> mesh);

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		ObjectPtr<Mesh> mesh_;							///< \brief 3D model.

	};

	/// \brief Basic class for camera components.
	/// \author Raffaele D. Facendola.
	class CameraComponent : public Component{

	public:
		
		/// \brief Create a new camera component.
		CameraComponent();

		/// \brief Virtual destructor.
		virtual ~CameraComponent();

		/// \brief Get the projection type.
		/// \return Returns the projection type.
		ProjectionType GetProjectionType() const;

		/// \brief Set the projection type.
		/// \param projection_type The new projection type.
		void SetProjectionType(ProjectionType projection_type);

		/// \brief Get the vertical field of view in radians.
		/// This value is only used when the projection type is "Perspective".
		/// \return Returns the vertical field of view in radians.
		float GetFieldOfView() const;

		/// \brief Set the veritical field of view.
		/// This value is only used when the projection type is "Perspective".
		/// \param field_of_view Vertical field of view, in radians.
		void SetFieldOfView(float field_of_view);

		/// \brief Get the near clipping plane distance.
		/// \return Returns the near clipping plane distance.
		float GetMinimumDistance() const;

		/// \brief Set the near clipping plane distance.
		/// \param minimum_distance The distance of the near clipping plane.
		void SetMinimumDistance(float minimum_distance);
		
		/// \brief Get the far clipping plane distance.
		/// \return Returns the far clipping plane distance.
		float GetMaximumDistance() const;

		/// \brief Set the far clipping plane distance.
		/// \param maximum_distance The distance of the far clipping plane.
		void SetMaximumDistance(float maximum_distance);

		/// \brief Get the view frustum.
		/// \param aspect_ratio Width-to-height aspect ratio.
		/// \return Returns the view frustum.
		Frustum GetViewFrustum(float aspect_ratio = 1.0f) const;

		/// \brief Get the view transform matrix.
		/// Use this matrix to convert from world-space to camera-space.
		/// \return Return the view-space matrix.
		Affine3f GetViewTransform() const;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		ProjectionType projection_type_;	///< \brief Projection type.

		float field_of_view_;				///< \brief Vertical field of view.

		float minimum_distance_;			///< \brief Near clipping plane distance.

		float maximum_distance_;			///< \brief Far clipping plane distance.

		TransformComponent* transform_;		///< \brief Transform component needed to compute the view frustum.

	};

	////////////////////////////////// SCENE //////////////////////////////

	inline const vector<NodeComponent*>& Scene::GetNodes() const{

		return nodes_;

	}

	inline vector<NodeComponent*>& Scene::GetNodes(){

		return nodes_;

	}

	////////////////////////////////// TRANSFORM COMPONENT ////////////////////////////////

	inline Vector3f TransformComponent::GetRight() const{
		
		return Math::ToVector3(GetLocalTransform().matrix().col(0)).normalized();

	}

	inline Vector3f TransformComponent::GetUp() const{

		return Math::ToVector3(GetLocalTransform().matrix().col(1)).normalized();
		
	}

	inline Vector3f TransformComponent::GetForward() const{

		return Math::ToVector3(GetLocalTransform().matrix().col(2)).normalized();

	}

	////////////////////////////////// CAMERA COMPONENT ///////////////////////////////////

	inline Affine3f CameraComponent::GetViewTransform() const{

		return transform_->GetWorldTransform().inverse();

	}

}