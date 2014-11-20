#include "..\include\components.h"

#include "..\include\scene.h"
#include "..\include\gimath.h"

using namespace gi_lib;
using namespace std;

///////////////////// NODE COMPONENT ///////////////////

NodeComponent::NodeComponent(SceneNode & node) :
	node_(node),
	enabled_(true){}

NodeComponent::~NodeComponent(){}

/////////////////////// BOUNDABLE ///////////////////////////////////////

Boundable::Boundable(SceneNode & node, const Bounds & bounds) :
NodeComponent(node),
bounds_(bounds){

	// Add the volume to the BVH
	Scene::GetInstance().GetBVH().AddVolume(*this);

}

Boundable::~Boundable(){

	// Remove the volume from the BVH
	Scene::GetInstance()
		  .GetBVH()
		  .RemoveVolume(*this);

}

void Boundable::SetBounds(const Bounds & bounds){

	bounds_ = bounds;

	// Notify the event to the observers.
	on_bounds_changed_.Notify(*this);

}

/////////////////////// GEOMETRY ///////////////////////////////////////

Geometry::Geometry(SceneNode & node, shared_ptr<Mesh> mesh) :
	Boundable(node, mesh->GetBounds()),
	mesh_(mesh),
	dirty_(true){}

void Geometry::PostUpdate(const Time &){

	// Update the bounds of the geometry 

	auto & node = GetNode();

	if (node.IsWorldTransformChanged() ||
		dirty_){

		SetBounds( mesh_->GetBounds().Transformed(node.GetWorldTransform()) );

		dirty_ = false;

	}
	

}

/////////////////////// RENDERER ///////////////////////////////////////

Renderer::Renderer(SceneNode & node) :
NodeComponent(node){

	

}

Renderer::~Renderer(){

}

/////////////////////// CAMERA /////////////////////////////////////////

Camera::Camera(SceneNode & node, shared_ptr<RenderTarget> target) :
NodeComponent(node),
target_(target){

	if (!target_){

		throw RuntimeException(L"Render target must not be null!");

	}

	projection_mode_ = ProjectionMode::kPerspective;
	clear_mode_ = ClearMode::kColor;

	viewport_.position = Vector2f::Zero();
	viewport_.extents = Vector2f::Ones();

	aspect_ratio_ = target_->GetAspectRatio();

	near_plane_ = 1.0f;
	far_plane_ = 1000.0f;

	clear_color_ = Color{ { 1.0f, 0.0f, 0.0f, 0.0f } };

	field_of_view_ = Math::DegToRad(60.0f);
	ortho_size_ = 1.0f;

}

void Camera::Update(const Time &){

	// The camera is likely to move every frame, so the view matrix must be always recomputed.
	UpdateViewMatrix();

	// The only external factor is the aspect ratio: if it changes the projection matrix must be recomputed.

	auto aspect_ratio = target_->GetAspectRatio();

	if (!Math::Equal(aspect_ratio, aspect_ratio_, 0.01f)){

		aspect_ratio_ = aspect_ratio;

		UpdateProjectionMatrix();

	}

}

Frustum Camera::GetViewFrustum() const{

	auto & projection_matrix = GetProjectionMatrix();

	Frustum frustum;

	// TODO: compute the actual frustum

	return frustum;

}

void Camera::UpdateProjectionMatrix(){

	if (projection_mode_ == ProjectionMode::kPerspective){

		// The matrix here is transposed: http://msdn.microsoft.com/en-us/library/bb205352(v=vs.85).aspx

		proj_matrix_ = Projective3f::Identity();

		auto tan_half_fov = std::tanf(field_of_view_ * 0.5f);

		proj_matrix_(0, 0) = 1.0f / (aspect_ratio_ * tan_half_fov);

		proj_matrix_(1, 1) = 1.0f / tan_half_fov;

		proj_matrix_(2, 2) = (far_plane_ + near_plane_) / (near_plane_ - far_plane_);

		proj_matrix_(2, 3) = (/* 2.0f * */ far_plane_ * near_plane_) / (near_plane_ - far_plane_);	// IMPORTANT: Under OpenGL, Multiply this by 2 (OpenGL has depth values ranging from -w to w. Direct3D uses [0;w] instead).

		proj_matrix_(3, 2) = /* -*/ 1;																// IMPORTANT: Left handed coordinate system. For right handed use -1 instead.

		proj_matrix_(3, 3) = 0.0f;
		
	}

	// Add code for the ortographic projection, eventually.

}

void Camera::UpdateViewMatrix(){

	// Naive approach: inverse of the world transformation applied to the camera.

	// ps: don't play with parent's scaling, otherwise the scene will look "squeezed" :D

	view_matrix_ = GetNode().GetWorldTransform().inverse();

}