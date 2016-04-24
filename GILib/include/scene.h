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
	
	class IStaticMesh;

	/// \brief Represents a scene and all its content.
	/// \author Raffaele D. Facendola
	class Scene{

	public:

		/// \brief Default constructor.
		Scene(unique_ptr<IVolumeHierarchy> mesh_hierarchy, unique_ptr<IVolumeHierarchy> light_hierarchy);

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

		/// \brief Get the mesh hierarchy.
		/// \return Returns the mesh hierarchy.
		IVolumeHierarchy& GetMeshHierarchy();

		/// \brief Get the volume hierarchy.
		/// \return Returns the volume hierarchy.
		const IVolumeHierarchy& GetMeshHierarchy() const;

		/// \brief Get the light hierarchy.
		/// \return Returns the light hierarchy
		IVolumeHierarchy& GetLightHierarchy();

		/// \brief Get the light hierarchy.
		/// \return Returns the light hierarchy/// \brief Get the light hierarchy.
		const IVolumeHierarchy& GetLightHierarchy() const;

		/// \brief Get the list of the nodes created so far.
		/// \return Returns the list of the nodes created so far.
		const vector<NodeComponent*>& GetNodes() const;

		/// \brief Get the list of the nodes created so far.
		/// \return Returns the list of the nodes created so far.
		vector<NodeComponent*>& GetNodes();

	private:

		vector<NodeComponent*> nodes_;						///< \brief Nodes inside the scene.

		CameraComponent* main_camera_;						///< \brief Main camera.

		unique_ptr<IVolumeHierarchy> mesh_hierarchy_;		///< \brief Mesh hierarchy.

		unique_ptr<IVolumeHierarchy> light_hierarchy_;		///< \brief Light hierarchy.

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
		/// The right direction defines the positive X-axis in world space.
		/// \return Returns the right direction.
		Vector3f GetRight() const;

		/// \brief Get the up direction.
		/// The up direction defines the positive Y-axis in world space.
		/// \return Returns the up direction.
		Vector3f GetUp() const;

		/// \brief Get the forward direction.
		/// The forward direction defines the positive Z-axis in world space.
		/// \return Returns the forward direction.
		Vector3f GetForward() const;

		/// \brief Get the position in world space.
		/// \return Returns the position in world space.
		Vector3f GetPosition() const;

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

		/// \brief Arguments relative to OnChanged event.
		struct OnChangedEventArgs{

			VolumeComponent* volume;	///< \brief Volume whose bounds have changed.

		};

		/// \brief Virtual destructor.
		virtual ~VolumeComponent() {};

		/// \brief Test the volume against the specified frustum.
		/// \param frustum The frustum being tested against.
		/// \return Returns the intersection type between this volume and the specified frustum from the volume's perspective.
		/// \remarks This is not intended to be a perfect test! False positive may occur.
		virtual IntersectionType TestAgainst(const Frustum& frustum) const = 0;

		/// \brief Test the volume against the specified axis-aligned bounding-box.
		/// \param box The box being tested against.
		/// \return Returns the intersection type between this volume and the specified box from the volume's perspective.
		/// \remarks This is not intended to be a perfect test! False positive may occur.
		virtual IntersectionType TestAgainst(const AABB& box) const = 0;

		/// \brief Test the volume against the specified sphere.
		/// \param sphere The sphere being tested against.
		/// \return Returns the intersection type between this volume and the specified sphere from the volume's perspective.
		/// \remarks This is not intended to be a perfect test! False positive may occur.
		virtual IntersectionType TestAgainst(const Sphere& sphere) const = 0;

		/// \brief Event that is signaled whenever the bounds change.
		/// \return Returns the event that is signaled whenever the bounds change.
		Observable<OnChangedEventArgs>& OnChanged();

		virtual TypeSet GetTypes() const override;

	protected:

		/// \brief Notify that the volume has changed
		void NotifyChange();

	private:

		Event<OnChangedEventArgs> on_changed_;					///< \brief Event signaled whenever the bounds change.

	};

	/// \brief Mesh component.
	/// \author Raffaele D. Facendola.
	class MeshComponent : public VolumeComponent{

	public:

		/// \brief Create an empty mesh component.
		MeshComponent();

		/// \brief Create a new mesh component.
		/// \param Static mesh associated to this component
		MeshComponent(ObjectPtr<IStaticMesh> mesh);
		
		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		ObjectPtr<IStaticMesh> GetMesh();

		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		ObjectPtr<const IStaticMesh> GetMesh() const;

		/// \brief Set the mesh associated to this component.
		/// \param mesh The new mesh to associate.
		void SetMesh(ObjectPtr<IStaticMesh> mesh);

		virtual IntersectionType TestAgainst(const Frustum& frustum) const override;

		virtual IntersectionType TestAgainst(const AABB& box) const override;

		virtual IntersectionType TestAgainst(const Sphere& sphere) const override;

		virtual TypeSet GetTypes() const override;

		/// \brief Get the world transform of the mesh.
		const Affine3f& GetWorldTransform() const;

		/// \brief Get the bounding sphere in world space.
		const Sphere& GetBoundingSphere() const;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		/// \brief Compute the new bounds of the mesh.
		/// \brief Whether the method will trigger the VolumeComponent::OnChanged event
		void ComputeBounds(bool notify = true);

		ObjectPtr<IStaticMesh> mesh_;							///< \brief 3D model.

		TransformComponent* transform_;							///< \brief Transform component needed to computed the transformed bounds.

		AABB bounding_box_;										///< \brief Bounding box.

		unique_ptr<Listener> on_transform_changed_lister_;		///< \brief Listener for the transform changed event.

		AABB transformed_bounds_;								///< \brief Transformed bounds.

		Sphere bounding_sphere_;								///< \brief Bounding sphere.

	};

	/// \brief Component used to bind each mesh with a material.
	/// The component will store one material per mesh subset.
	/// \tparam TMaterial Type of the material stored per mesh.
	template <typename TMaterial>
	class AspectComponent : public Component {

	public:

		/// \brief No default constructor.
		AspectComponent() = delete;

		/// \brief Create a new deferred renderer component.
		/// \param mesh_component Mesh component to draw.		
		AspectComponent(MeshComponent& mesh_component);

		/// \brief No assignment operator.
		AspectComponent& operator=(const AspectComponent&) = delete;

		/// \brief Virtual destructor.
		virtual ~AspectComponent();

		/// \brief Get the mesh component.
		/// \return Returns the mesh component.
		ObjectPtr<IStaticMesh> GetMesh();

		/// \brief Get the mesh component.
		/// \return Returns the mesh component.
		ObjectPtr<const IStaticMesh> GetMesh() const;

		/// \brief Get the material count.
		/// \return Returns the material count.
		unsigned int GetMaterialCount() const;

		/// \brief Get a material.
		/// \param material_index Index of the material to retrieve.
		/// \return Returns the specified material.
		ObjectPtr<TMaterial> GetMaterial(unsigned int material_index);

		/// \brief Get a material.
		/// \param material_index Index of the material to retrieve.
		/// \return Returns the specified material.
		ObjectPtr<const TMaterial> GetMaterial(unsigned int material_index) const;

		/// \brief Get the world transform.
		/// \return Return the world transform associated to the transform component.
		const Affine3f& GetWorldTransform() const;

		/// \brief Set a new material.
		/// \param material_index Index of the material to set.
		/// \param material New material
		void SetMaterial(unsigned int material_index, ObjectPtr<TMaterial> material);

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		MeshComponent& mesh_component_;									///< \brief Mesh component to draw (must belong to the same object)

		TransformComponent* transform_component_;						///< \brief Transform component used to draw the mesh (must belong to the same object)

		unique_ptr<Listener> on_mesh_removed_listener_;					///< \brief Detects whether the mesh component have been removed from the object.

		vector<ObjectPtr<TMaterial>> materials_;						///< \brief List of materials (one per mesh subset)

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

		/// \brief Get the vertical orthographic size in world units.
		/// This value is only used when the projection type is "Orthographic".
		/// \return Returns the vertical orthographic size in world units.
		float GetOrthoSize() const;

		/// \brief Set the vertical orthographic size.
		/// This value is only used when the projection type is "Orthographic".
		/// \param ortho_size Vertical orthographic size in world units.
		void SetOrthoSize(float ortho_size);

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

		/// \brief Get the world transform of the camera.
		/// \return Returns the world transform of the camera.
		const Affine3f& GetWorldTransform() const;

		virtual TypeSet GetTypes() const override;

		/// \brief Clone the component inside another one.
		void Clone(CameraComponent& target) const;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		ProjectionType projection_type_;				///< \brief Projection type.

		union {

			float field_of_view_;						///< \brief Vertical field of view.

			float ortho_size_;							///<\ brief Vertical orthographic size.

		};
		
		float minimum_distance_;						///< \brief Near clipping plane distance.

		float maximum_distance_;						///< \brief Far clipping plane distance.

		Affine3f transform_;							///< \brief World transform matrix.

		TransformComponent* transform_component_;		///< \brief Transform component needed to compute the view frustum.
		
	};

	////////////////////////////////// SCENE //////////////////////////////

	inline const vector<NodeComponent*>& Scene::GetNodes() const{

		return nodes_;

	}

	inline vector<NodeComponent*>& Scene::GetNodes(){

		return nodes_;

	}

	////////////////////////////////// VOLUME COMPONENT ///////////////////////////////////

	inline Observable<VolumeComponent::OnChangedEventArgs>& VolumeComponent::OnChanged() {

		return on_changed_;

	}

	inline void VolumeComponent::NotifyChange() {

		auto args = OnChangedEventArgs{ this };

		on_changed_.Notify(args);

	}

	////////////////////////////////// MESH COMPONENT //////////////////////////////////////

	inline const Affine3f& MeshComponent::GetWorldTransform() const {

		return transform_->GetWorldTransform();

	}

	inline const Sphere& MeshComponent::GetBoundingSphere() const {

		return bounding_sphere_;

	}

	////////////////////////////////// TRANSFORM COMPONENT ////////////////////////////////

	inline Vector3f TransformComponent::GetRight() const{
		
		return Math::ToVector3(GetWorldTransform().matrix().col(0)).normalized();

	}

	inline Vector3f TransformComponent::GetUp() const{

		return Math::ToVector3(GetWorldTransform().matrix().col(1)).normalized();
		
	}

	inline Vector3f TransformComponent::GetForward() const{

		return Math::ToVector3(GetWorldTransform().matrix().col(2)).normalized();

	}

	inline Vector3f TransformComponent::GetPosition() const {

		return Math::ToVector3(GetWorldTransform().matrix().col(3));

	}

	////////////////////////////////// DEFERRED RENDERER COMPONENT //////////////////////////////////

	template <typename TMaterial>
	AspectComponent<TMaterial>::AspectComponent(MeshComponent& mesh_component) :
		mesh_component_(mesh_component) {

		// If the mesh component gets removed, this component is removed as well.

		on_mesh_removed_listener_ = mesh_component.OnRemoved().Subscribe([this](Listener& listener, Component::OnRemovedEventArgs&) {

			listener.Unsubscribe();

			RemoveComponent();

		});

		// One material per mesh subset.

		materials_.resize(mesh_component_.GetMesh()->GetSubsetCount());

	}

	template <typename TMaterial>
	AspectComponent<TMaterial>::~AspectComponent() {}

	template <typename TMaterial>
	inline ObjectPtr<IStaticMesh> AspectComponent<TMaterial>::GetMesh() {

		return ObjectPtr<IStaticMesh>(mesh_component_.GetMesh());

	}

	template <typename TMaterial>
	inline ObjectPtr<const IStaticMesh> AspectComponent<TMaterial>::GetMesh() const {

		return ObjectPtr<const IStaticMesh>(mesh_component_.GetMesh());

	}

	template <typename TMaterial>
	inline unsigned int AspectComponent<TMaterial>::GetMaterialCount() const {

		return static_cast<unsigned int>(mesh_component_.GetMesh()->GetSubsetCount());

	}

	template <typename TMaterial>
	inline ObjectPtr<TMaterial> AspectComponent<TMaterial>::GetMaterial(unsigned int material_index) {

		return materials_[material_index];

	}

	template <typename TMaterial>
	inline ObjectPtr<const TMaterial> AspectComponent<TMaterial>::GetMaterial(unsigned int material_index) const {

		return ObjectPtr<const TMaterial>(materials_[material_index]);

	}

	template <typename TMaterial>
	inline void AspectComponent<TMaterial>::SetMaterial(unsigned int material_index, ObjectPtr<TMaterial> material) {

		materials_[material_index] = material;

	}

	template <typename TMaterial>
	typename AspectComponent<TMaterial>::TypeSet AspectComponent<TMaterial>::GetTypes() const {

		auto types = Component::GetTypes();

		types.insert(type_index(typeid(AspectComponent<TMaterial>)));

		return types;

	}

	template <typename TMaterial>
	inline void AspectComponent<TMaterial>::Initialize() {

		transform_component_ = GetComponent<TransformComponent>();

	}

	template <typename TMaterial>
	inline void AspectComponent<TMaterial>::Finalize() {

		transform_component_ = nullptr;

	}

	template <typename TMaterial>
	inline const Affine3f& AspectComponent<TMaterial>::GetWorldTransform() const {

		return transform_component_->GetWorldTransform();

	}

	////////////////////////////////// CAMERA COMPONENT ///////////////////////////////////

	inline Affine3f CameraComponent::GetViewTransform() const{

		return GetWorldTransform().inverse();

	}

	inline const Affine3f& CameraComponent::GetWorldTransform() const {

		return transform_component_ ?
			   transform_component_->GetWorldTransform() :
			   transform_;

	}

}