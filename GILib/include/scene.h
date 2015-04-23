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
#include "resources.h"
#include "graphics.h"

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
	class NodeComponent;
	class TransformComponent;

	/// \brief Represents a scene and all its content.
	/// \author Raffaele D. Facendola
	class Scene : public Component{

	public:

		/// \brief Default constructor.
		Scene();

		/// \brief No copy constructor.
		Scene(const Scene&) = delete;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:



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
	/// The composite tranformation is calculated by applying the scaling first, the rotation second and the translation last.
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

		/// \brief Get the local transfom.
		/// \return Returns the local transform matrix.
		const Affine3f & GetLocalTransform() const;

		/// \brief Get the global transfom.
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
		/// \return Returns the transformed bouding box.
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

	/// \brief Static mesh component.
	/// \author Raffaele D. Facendola.
	class StaticMeshComponent : public VolumeComponent{

	public:

		/// \brief Create an empty static mesh component.
		StaticMeshComponent();

		/// \brief Create a new static mesh component.
		/// \param Static mesh associated to this component
		StaticMeshComponent(shared_ptr<Mesh> mesh);
		
		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		shared_ptr<Mesh> GetMesh();

		/// \brief Get the mesh associated to this component.
		/// \return Return the mesh associated to this component.
		shared_ptr<const Mesh> GetMesh() const;

		/// \brief Set the mesh associated to this component.
		/// \param mesh The new mesh to associate.
		void SetMesh(shared_ptr<Mesh> mesh);

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		shared_ptr<Mesh> mesh_;		///< \brief Static mesh.

	};

	/// \brief Basic class for camera components.
	/// \author Raffaele D. Facendola.
	class CameraComponent : public Component{

	public:

		/// \brief Clear mode for the render target.
		enum class ClearMode{

			kNone,								///< \brief Do no clear.
			kDepthOnly,							///< \brief Clear the depth buffer and only.
			kColor,								///< \brief Clear the depth buffer and the color buffer.

		};

		/// \brief Create a new camera component.
		CameraComponent();

		/// \brief Virtual destructor.
		virtual ~CameraComponent();

		/// \brief Get the viewport.
		/// \return Returns the viewport.
		Viewport GetViewport() const;

		/// \brief Set the viewport.
		/// \param viewport The new viewport.
		void SetViewport(const Viewport & viewport);

		/// \brief Get the near clipping plane.
		/// \return Returns the near clipping plane in world units.
		float GetMinimumDistance() const;

		/// \brief Set the near clipping plane.
		/// \param distance The near clipping plane in world units. 
		void SetMinimumDistance(float distance);

		/// \brief Get the far clipping plane.
		/// \return Returns the far clipping plane in world units.
		float GetMaximumDistance() const;

		/// \brief Get the far clipping plane.
		/// \return Returns the far clipping plane in world units.
		void SetMaximumDistance(float distance);

		/// \brief Get the clear mode.
		/// \return Returns the clear mode.
		ClearMode GetClearMode() const;

		/// \brief Set the clear mode.
		/// \param clear_mode The new clear mode.
		void SetClearMode(ClearMode clear_mode);

		/// \brief Get the clear color.
		/// This value has an actual meaning only if the clear mode is kColor.
		/// \returns The clear color.
		Color GetClearColor() const;

		/// \brief Set the clear color.
		/// This value has an actual meaning only if the clear mode is kColor.
		/// \param color The new clear color.
		void SetClearColor(Color color);

		/// \brief Get the clear depth.
		/// This value has an actual meaning only if the clear mode is kDepthOnly or kColor.
		/// \return Returns the clear depth normalized in the range 0 (minimum distance) and 1 (maximum distance).
		float GetClearDepth() const;

		/// \brief Set the clear depth.
		/// This value has an actual meaning only if the clear mode is kDepthOnly or kColor.
		/// \param depth The new clear depth normalized in the range 0 (minimum distance) and 1 (maximum distance). Values outside this range are clamped.
		void SetClearDepth(float depth);

		virtual Frustum GetViewFrustum() const = 0;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		shared_ptr<RenderTarget> render_target_;

	};

	class PerspectiveCameraComponent : public CameraComponent{

	public:

		/// \brief Get the vertical field of view in radians.
		/// \return Return the field of view in radians.
		float GetFieldOfView() const;

		/// \brief Set the vertical field of view in randians.
		/// \param fov The new vertical field of view in radians.
		void SetFieldOfView(float fov);

	};



}