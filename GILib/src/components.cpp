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
	GetNode().GetScene().GetBVH().AddBoundable(*this);

}

Boundable::~Boundable(){

	// Remove the volume from the BVH
	GetNode().GetScene().GetBVH().RemoveBoundable(*this);

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

/////////////////////// ASPECT ///////////////////////////////////////

Aspect::Aspect(SceneNode & node) :
NodeComponent(node){}

Aspect::~Aspect(){}

/////////////////////// CAMERA /////////////////////////////////////////

Camera::Camera(SceneNode & node, shared_ptr<RenderTarget> target) :
NodeComponent(node),
target_(target){

	if (!target_){

		THROW(L"Render target must not be null!");

	}

	projection_mode_ = ProjectionMode::kPerspective;
	clear_mode_ = ClearMode::kColor;

	// The entire render surface.
	viewport_.position = Vector2f::Zero();		
	viewport_.extents = Vector2f::Ones();

	aspect_ratio_ = target_->GetAspectRatio();

	near_plane_ = 1.0f;
	far_plane_ = 1000.0f;

	// Solid black
	clear_color_ = Color{ { 0.0f, 0.0f, 0.0f, 0.0f } };

	field_of_view_ = Math::DegToRad(60.0f);
	ortho_size_ = 1.0f;

	priority_ = 0;

	//Add the camera to the sorted list
	GetNode().GetScene().AddCamera(*this);

}

Camera::~Camera(){

	// Remove this camera from the camera list
	GetNode().GetScene().RemoveCamera(*this);

}

void Camera::SetPriority(int priority){

	priority_ = priority;

	GetNode().GetScene().SortCamerasByPriority();

}

void Camera::Update(const Time &){

	// Update the aspect ratio

	aspect_ratio_ = target_->GetAspectRatio();

}

Frustum Camera::GetViewFrustum() const{
	
	auto view_matrix = GetNode().GetWorldTransform().inverse();	//View matrix

	// Projection matrix (Using D3D left-handed notation, should not change the final result though)

	auto proj_matrix = Projective3f::Identity();

	if (projection_mode_ == ProjectionMode::kPerspective){

		auto cot_half_FoV = 1.0f / std::tanf(field_of_view_ * 0.5f);

		proj_matrix(0, 0) = cot_half_FoV / aspect_ratio_;

		proj_matrix(1, 1) = cot_half_FoV;

		proj_matrix(2, 2) = far_plane_ / (far_plane_ - near_plane_);

		proj_matrix(2, 3) = -(far_plane_ * near_plane_) / (far_plane_ - near_plane_);

		proj_matrix(3, 2) = 1.0f;

		proj_matrix(3, 3) = 0.0f;

	}
	else{

		// Add the orthographic projection matrix here
		THROW(L"Not yet implemented, duh!");

	}
	
	// Compute the frustum from view-proj matrix

	// Fancy details here: http://www.chadvernon.com/blog/resources/directx9/frustum-culling/
	// more: http://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/

	auto view_proj_matrix = (proj_matrix * view_matrix).matrix();

	Frustum frustum;

	frustum.planes[0] = view_proj_matrix.row(3) + view_proj_matrix.row(0);		// Left
	frustum.planes[1] = view_proj_matrix.row(3) - view_proj_matrix.row(0);		// Right
	frustum.planes[2] = view_proj_matrix.row(3) + view_proj_matrix.row(1);		// Bottom
	frustum.planes[3] = view_proj_matrix.row(3) - view_proj_matrix.row(1);		// Top
	frustum.planes[4] = view_proj_matrix.row(2);								// Near
	frustum.planes[5] = view_proj_matrix.row(3) - view_proj_matrix.row(2);		// Far

	return frustum;

}