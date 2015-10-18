#include "gimath.h"

#include <math.h>

#include "exceptions.h"

using namespace ::gi_lib;
using namespace ::Eigen;

float Math::kPi = 3.1415926f;

float Math::kDegToRad = Math::kPi / 180.0f;

float Math::kRadToDeg = 1.0f / Math::kDegToRad;

///////////////////////////////////////// AABB /////////////////////////////////////////

AABB AABB::operator*(const Affine3f& transform) const{

	/// Theory on: http://dev.theomader.com/transform-bounding-boxes/

	Matrix4f matrix = transform.matrix();

	Vector3f min = center - half_extents;
	Vector3f max = center + half_extents;

	Vector3f min_transformed = Math::ToVector3(matrix.col(3));
	Vector3f max_transformed = Math::ToVector3(matrix.col(3));

	Vector3f a, b, col;

	for (int i = 0; i < 3; i++){

		col = Math::ToVector3(matrix.col(i));

		a = col * min(i);
		b = col * max(i);

		min_transformed += Math::Min(a, b);
		max_transformed += Math::Max(a, b);

	}

	return AABB{ 0.5f * (max_transformed + min_transformed),
				 0.5f * (max_transformed - min_transformed) };
		
}

IntersectionType AABB::Intersect(const AABB& other) const{

	float diff[3];

	if ((diff[0] = std::fabs(other.center(0) - center(0)) - half_extents(0)) >= other.half_extents(0) ||
		(diff[1] = std::fabs(other.center(1) - center(1)) - half_extents(1)) >= other.half_extents(1) ||
		(diff[2] = std::fabs(other.center(2) - center(2)) - half_extents(2)) >= other.half_extents(2)){

		return IntersectionType::kNone;		// Strictly separated

	}

	if (diff[0] < -other.half_extents(0) &&
		diff[1] < -other.half_extents(1) &&
		diff[2] < -other.half_extents(2)){

		return IntersectionType::kInside;	// Strictly enclosed

	}

	return IntersectionType::kIntersect;	// Intersection

}

///////////////////////////////////////// SPHERE /////////////////////////////////////////

Sphere Sphere::FromAABB(const AABB& aabb){

	return Sphere{ aabb.center ,
				   aabb.half_extents.norm() };	// Implies a square root.

}

///////////////////////////////////////// FRUSTUM /////////////////////////////////////////

Frustum::Frustum(const vector<Vector4f>& planes){

	if (planes.size() != 6){

		THROW(L"A frustum must have exactly 6 planes!");

	}

	Vector3f normal;

	for (int plane_index = 0; plane_index < 6; ++plane_index){

		normal = Math::ToVector3(planes[plane_index]).normalized();

		planes_[plane_index] = Vector4f(normal(0), normal(1), normal(2), planes[plane_index](3));

		abs_normals_[plane_index] = normal.cwiseAbs();

	}
	
}

IntersectionType Frustum::Intersect(const AABB& bounds) const{

	/// Theory on: https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
	/// Method 5 with a small tweak: the w component is kept within the plane and the dot product will multiply it by 1.

	int plane_index = 0;

	auto hcenter = Math::ToHomogeneous(bounds.center);	// Needed to compute the distance as a dot product between two 4-elements vectors.
	
	for (auto& plane : planes_){

		if (plane.dot(hcenter) < -bounds.half_extents.dot(abs_normals_[plane_index])){

			return IntersectionType::kNone;			// Outside the plane

		}
		
		++plane_index;

	}

	return IntersectionType::kIntersect;	// Intersection

}

IntersectionType Frustum::Intersect(const Sphere& sphere) const{

	auto hcenter = Math::ToHomogeneous(sphere.center);	// Needed to compute the distance as a dot product between two 4-elements vectors.

	for (auto& plane : planes_){

		if (plane.dot(hcenter) < -sphere.radius ){

			return IntersectionType::kNone;			// Outside the plane

		}

	}

	return IntersectionType::kIntersect;

}