/// \file components.h
/// \brief Defines the base classes used to manage the scene node components.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "graphics.h"
#include "resources.h"
#include "gimath.h"
#include "timer.h"
#include "maybe.h"
#include "observable.h"

using ::Eigen::Affine3f;
using ::Eigen::Translation3f;
using ::Eigen::AlignedScaling3f;
using ::Eigen::Quaternionf;
using ::std::vector;
using ::std::shared_ptr;
using ::std::reference_wrapper;

namespace gi_lib{

	/// \brief Scene object component.
	/// \author Raffaele D. Facendola
	class NodeComponent{

		friend class SceneNode;

	public:

		/// \brief Create a new node component.
		/// \param node The node this component belongs to.
		NodeComponent(SceneNode & node);

		/// \brief No copy constructor.
		NodeComponent(const NodeComponent & other) = delete;

		/// \brief No assignment operator.
		NodeComponent & operator=(const NodeComponent & other) = delete;

		virtual ~NodeComponent();

		/// \brief Get the node this component belongs to.

		/// \return Returns the component's owner reference.
		SceneNode & GetNode();

		/// \brief Get the node this component belongs to.

		/// \return Returns the component's owner reference.
		const SceneNode & GetNode() const;

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

		/// \brief Method called after all Update methods have been called.
		
		/// The method is intended to update the internal state of the component based on the state of others.
		/// Avoid cross-component update here!
		/// \param time The current time application time.
		virtual void PostUpdate(const Time & time);

	private:

		bool enabled_;

		SceneNode & node_;

	};

	/// \brief Contains informations about an axis-aligned bounding box surrounding the node.
	class Boundable : public NodeComponent{

	public:

		/// \brief Create a new boundable component.
		Boundable(SceneNode & node, const Bounds & bounds);

		virtual ~Boundable();

		/// \brief Get the updated mesh bounds.
		/// \return Returns the updated bounds of the mesh.
		Bounds GetBounds() const ;

		/// \brief Convert the boundable component to actual bounds.
		operator const Bounds &() const;

		/// \brief Event raised whenever the bounds associated to this component change.
		/// \return Returns a reference to the observable event raised after the bounds have changed.
		Observable<Boundable&> & OnBoundsChanged();
		
	protected:
		
		virtual void Update(const Time & time) = 0;

		/// \brief Set the bounds for this component.
		/// \param bounds The new bounds.
		void SetBounds(const Bounds & bounds);

	private:

		Bounds bounds_;

		Event<Boundable &> on_bounds_changed_;

	};

	/// \brief Contains informations about a geometry.
	class Geometry : public Boundable{

	public:

		/// \brief Create a new geometry component.
		/// \param node The node this component belongs to.
		/// \param mesh Pointer to the mesh bound to this component.
		Geometry(SceneNode & node, shared_ptr<Mesh> mesh);

		/// \brief Set a new mesh.
		/// \param mesh Pointer to the mesh to set.
		void SetMesh(const shared_ptr<Mesh> & mesh);

		/// \brief Get a pointer to this component's mesh.
		/// \return Return a pointer to this component's mesh.
		shared_ptr<Mesh> GetMesh();

		/// \brief Get a pointer to this component's mesh.
		/// \return Returns a pointer to this component's mesh.
		shared_ptr<const Mesh> GetMesh() const;
		
	protected:

		virtual void Update(const Time & time);

		virtual void PostUpdate(const Time & time) override;

	private:

		shared_ptr<Mesh> mesh_;

		bool dirty_;									///< \brief Flag used to determine whether the mesh changed.
		
	};

	/// \brief Component used to display objects on screen.
	/// This componenent requires any geometry component to be attached to the node!
	class Renderer : public NodeComponent{

	public:

		Renderer(SceneNode & node);

		~Renderer();

		/// \brief Get the bounds of the mesh in world space.
		/// \return Returns the bounds of the mesh in world space.
		Bounds GetBounds() const;

		/// \brief Get the materials' vector.
		/// \return Returns the materials' vector.
		vector<shared_ptr<Material>> & GetMaterials();

		/// \brief Set the materials' vector.
		/// \param materials The new materials' vector.
		void SetMaterials(const vector<shared_ptr<Material>> & materials);

		/// \brief Get the materials' vector.
		/// \return Returns the materials' vector.
		const vector<shared_ptr<Material>> & GetMaterials() const;

	protected:

		virtual void Update(const Time & time) override;

	private:

		vector<shared_ptr<Material>> materials_;

	};

	/// \brief Component used to display the scene.
	
	/// The position of the camera is given by the transform component of the node this camera is attached to.
	class Camera : public NodeComponent{

	public:

		/// \brief Projection mode.
		enum class ProjectionMode{

			kPerspective,						///< \brief Perspective projection.
			//kOrthographic,						///< \brief Orthographic projection.

		};

		/// \brief Clear settings.
		enum class ClearMode{

			kNone,								///< \brief Do no clear.
			kDepthOnly,							///< \brief Clear the depth buffer only.
			kColor,								///< \brief Clear the depth buffer and the color buffer.

		};

		/// \brief Default constructor.

		/// The default camera is perspective
		/// \param node The node this component belongs to.
		/// \param target The render target of the camera.
		Camera(SceneNode & node, shared_ptr<RenderTarget> target);

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
			
		/// \brief Get the current view frustum.
		/// \return Return the current view frustum.
		Frustum GetViewFrustum() const;

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

	};

	// Node component

	inline SceneNode & NodeComponent::GetNode(){

		return node_;

	}

	inline const SceneNode & NodeComponent::GetNode() const{

		return node_;

	}

	inline bool NodeComponent::IsEnabled() const{

		return enabled_;

	}

	inline void NodeComponent::SetEnabled(bool enabled){

		enabled_ = enabled;

	}

	inline void NodeComponent::PostUpdate(const Time &){}

	// Boundable

	inline Bounds Boundable::GetBounds() const{

		return bounds_;

	}

	inline Observable<Boundable&> & Boundable::OnBoundsChanged(){

		return on_bounds_changed_;

	}

	inline Boundable::operator const Bounds &() const{

		return bounds_;

	}

	// Geometry

	inline void Geometry::SetMesh(const shared_ptr<Mesh> & mesh){

		mesh_ = mesh;
		dirty_ = true;

	} 

	inline shared_ptr<Mesh> Geometry::GetMesh(){

		return mesh_;

	}
		
	inline shared_ptr<const Mesh> Geometry::GetMesh() const{

		return std::static_pointer_cast<const Mesh>(mesh_);

	}

	inline void Geometry::Update(const Time &){}

	// Renderer

	inline vector<shared_ptr<Material>> & Renderer::GetMaterials(){

		return materials_;

	}

	inline const vector<shared_ptr<Material>> & Renderer::GetMaterials() const{

		return materials_;

	}

	inline void Renderer::SetMaterials(const vector<shared_ptr<Material>> & materials){

		materials_ = materials;

	}

	inline void Renderer::Update(const Time &){}

	// Camera

	inline Camera::ProjectionMode Camera::GetProjectionMode() const{

		return projection_mode_;

	}

	inline void Camera::SetProjectionMode(ProjectionMode projection_mode){

		projection_mode_ = projection_mode;

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

	}

	inline float Camera::GetFarPlane() const{

		return far_plane_;

	}

	inline void Camera::SetFarPlane(float far_plane){

		far_plane_ = far_plane;

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

	}

	inline float Camera::GetOrthoSize(){

		return ortho_size_;

	}

	inline void Camera::SetOrthoSize(float ortho_size){

		ortho_size_ = ortho_size;

	}
	
}
