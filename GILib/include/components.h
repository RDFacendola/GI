/// \file components.h
/// \brief Defines the base classes used to manage the scene node components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>

#include "graphics.h"
#include "resources.h"
#include "gimath.h"
#include "timer.h"
#include "maybe.h"

using ::Eigen::Affine3f;
using ::std::vector;
using ::std::shared_ptr;

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

	/// \brief Contains informations about static geometry.
	class StaticGeometry : public NodeComponent{

	public:

		/// \brief Create a new geometry component.
		/// \param mesh Pointer to the mesh bound to this component.
		StaticGeometry(const shared_ptr<Mesh> & mesh);

		/// \brief Set a new mesh.
		/// \param mesh Pointer to the mesh to set.
		void SetMesh(const shared_ptr<Mesh> & mesh);

		/// \brief Get a pointer to this component's mesh.
		/// \return Return a pointer to this component's mesh.
		shared_ptr<Mesh> GetMesh();

		/// \brief Get a pointer to this component's mesh.
		/// \return Return a pointer to this component's mesh.
		shared_ptr<const Mesh> GetMesh() const;

	protected:

		/// \brief Update the component.

		/// \param time The current application time.
		virtual void Update(const Time & time);

	private:

		shared_ptr<Mesh> mesh_;
		
	};

	/// \brief Component used to display objects on screen.
	class Renderer : public NodeComponent{

	};

	/// \brief Component used to display the scene.
	
	/// The position of the camera is given by the transform component of the node this camera is attached to.
	class Camera : public NodeComponent{

	public:

		/// \brief Projection mode.
		enum class ProjectionMode{

			kPerspective,						///< \brief Perspective projection.
			kOrthographic,						///< \brief Orthographic projection.

		};

		/// \brief Clear settings.
		enum class ClearMode{

			kNone,								///< \brief Do no clear.
			kDepthOnly,							///< \brief Clear the depth buffer only.
			kColor,								///< \brief Clear the depth buffer and the color buffer.

		};

		/// \brief Default constructor.

		/// The default camera is perspective
		/// \param target The render target of the camera.
		Camera(shared_ptr<RenderTarget> target);

		/// \brief Get the projection mode.
		/// \return Returns the projection mode.
		ProjectionMode GetProjectionMode() const;

		/// \brief Set the projection mode.
		/// \param projection_mode The new projection mode.
		void SetProjectionMode(ProjectionMode projection_mode);

		/// \brief Get the clear mode.
		/// \return Returns the clear mode.
		ClearMode GetClearMode() const;

		/// \brief Set the clear mode.
		/// \param clear_mode The new clear mode.
		void SetClearMode(ClearMode clear_mode);

		/// \brief Get the render target.
		/// \return Returns the render target.
		shared_ptr<RenderTarget> GetRenderTarget();

		/// \brief Get the render target.
		/// \return Returns the render target.
		shared_ptr<const RenderTarget> GetRenderTarget() const;

		/// \brief Get the camera viewport.
		/// \return Returns the camera viewport.
		Viewport GetViewport() const;

		/// \brief Set the camera viewport.
		/// \param viewport The new viewport value.
		void SetViewport(const Viewport & viewport);

		/// \brief Get the camera aspect ratio.

		/// The aspect ratio is Width/Height
		/// \return Returns the camera aspect ratio.
		float GetAspectRatio() const;

		/// \brief Get the near plane distance.
		/// \return Returns the near plane distance.
		float GetNearPlane() const;

		/// \brief Set the near plane distance.
		/// \param near_plane The new near plane distance.
		void SetNearPlane(float near_plane);

		/// \brief Get the far plane distance.
		/// \return Returns the far plane distance.
		float GetFarPlane() const;

		/// \brief Set the far plane distance.
		/// \param far_plane The new far plane distance.
		void SetFarPlane(float far_plane);

		/// \brief Get the color used to clear the target.

		/// This property is used only when the clear mode is "Color".
		/// \return Returns the color used to clear the target.
		Color GetClearColor() const;

		/// \brief Set the color used to clear the target.

		/// This property is used only when the clear mode is "Color".
		/// \param color Set the new clear color.
		void SetClearColor(Color color);

		/// \brief Get the field of view in radians.

		/// This property is only used when the projection mode is "Perspective".
		/// \return Return the field of view in radians.
		float GetFieldOfView() const;

		/// \brief Set the field of view in randians.

		/// This property is used only when the projection mode is "Perspective".
		/// \param field_of_view The new field of view in radians.
		void SetFieldOfView(float field_of_view);

		/// \brief Get the height of the viewing volume.

		/// This property is used only when the projection mode is "Orthographic".
		/// \return Return the height of the viewing volume.
		float GetOrthoSize();

		/// \brief Set the height of the viewing volume.

		/// This property is used only when the projection mode is "Orthographic".
		/// \param ortho_size The new orthographic size.
		void SetOrthoSize(float ortho_size);
			
		/// \brief Get the current view matrix.
		/// \return Return the view matrix.
		Affine3f GetViewMatrix();

		/// \brief Get the current projection matrix.
		/// \return Return the projection matrix.
		Projective3f GetProjectionMatrix();

	protected:

		/// \brief Update the component.

		/// \param time The current application time.
		virtual void Update(const Time & time);

	private:

		ProjectionMode projection_mode_;		///< \brief Projection mode of this camera.

		ClearMode clear_mode_;					/// < \brief The clear mode of this camera.

		shared_ptr<RenderTarget> target_;		///< \brief Surface(s) the scene will be displayed onto.

		Viewport viewport_;						///< \brief Region of the target the camera will display the image to.

		float aspect_ratio_;					///< \brief Width to height ratio of the surface the camera will render to.

		float near_plane_;						///< \brief Near clipping plane's distance.

		float far_plane_;						///< \brief Far clipping plane's distance.

		// Clear mode-specific parameters
		union{

			Color clear_color_;					///< \brief Color that will be used to clear the target. Valid when clear mode is "Color".

		};

		// Projection-specific parameters
		union{
				
			float field_of_view_;				///< \brief Vertical FoV in radians. Valid when projection mode is "Perspective".
			float ortho_size_;					///< \brief Units of the scene the camera can display vertically. Valid when projection mode is "Orthographic".

		};

		bool projection_dirty_ = true;							///< \brief Is the projection matrix dirty?

		bool view_dirty_ = true;								///< \brief Is the view matrix dirty?

		Affine3f view_matrix_;									///< \brief The camera matrix.

		Projective3f proj_matrix_;								///< \brief The projection matrix.

		void UpdateProjectionMatrix(bool force = false);		///< \brief Update the projection matrix.

		void UpdateViewMatrix(bool force = false);				///< \brief Update the view matrix.

	};

	// Node component

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

	// Transform

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
	
	//

	inline StaticGeometry::StaticGeometry(const shared_ptr<Mesh> & mesh):
		mesh_(mesh){}


	inline void StaticGeometry::SetMesh(const shared_ptr<Mesh> & mesh){

		mesh_ = mesh;

	} 

	inline shared_ptr<Mesh> StaticGeometry::GetMesh(){

		return mesh_;

	}
		
	inline shared_ptr<const Mesh> StaticGeometry::GetMesh() const{

		return std::static_pointer_cast<const Mesh>(mesh_);

	}

	inline void StaticGeometry::Update(const Time &){}

	// Renderer

	// Camera

	inline Camera::ProjectionMode Camera::GetProjectionMode() const{

		return projection_mode_;

	}

	inline void Camera::SetProjectionMode(ProjectionMode projection_mode){

		projection_mode_ = projection_mode;

		projection_dirty_ = true;

	}

	inline Camera::ClearMode Camera::GetClearMode() const{

		return clear_mode_;

	}

	inline void Camera::SetClearMode(ClearMode clear_mode){

		clear_mode_ = clear_mode;

	}

	inline shared_ptr<RenderTarget> Camera::GetRenderTarget(){

		return target_;

	}

	inline shared_ptr<const RenderTarget> Camera::GetRenderTarget() const{

		return target_;

	}

	inline Viewport Camera::GetViewport() const{
		
		return viewport_;

	}

	inline void Camera::SetViewport(const Viewport & viewport){

		viewport_ = viewport;

	}

	inline float Camera::GetAspectRatio() const{

		return aspect_ratio_;

	}

	inline float Camera::GetNearPlane() const{

		return near_plane_;

	}

	inline void Camera::SetNearPlane(float near_plane){

		near_plane_ = near_plane;

		projection_dirty_ = true;

	}

	inline float Camera::GetFarPlane() const{

		return far_plane_;

	}

	inline void Camera::SetFarPlane(float far_plane){

		far_plane_ = far_plane;

		projection_dirty_ = true;

	}

	inline Color Camera::GetClearColor() const{

		return clear_color_;

	}

	inline void Camera::SetClearColor(Color color){

		clear_color_ = color;

	}
	
	inline float Camera::GetFieldOfView() const{

		return field_of_view_;

	}
	
	inline void Camera::SetFieldOfView(float field_of_view){

		field_of_view_ = field_of_view;

		projection_dirty_ = true;

	}

	inline float Camera::GetOrthoSize(){

		return ortho_size_;

	}

	inline void Camera::SetOrthoSize(float ortho_size){

		ortho_size_ = ortho_size;

		projection_dirty_ = true;

	}
	
}
