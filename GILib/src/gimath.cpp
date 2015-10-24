#include "gimath.h"

#include <math.h>

#include "exceptions.h"

using namespace ::gi_lib;
using namespace ::Eigen;

float Math::kPi = 3.1415926f;

float Math::kDegToRad = Math::kPi / 180.0f;

float Math::kRadToDeg = 1.0f / Math::kDegToRad;

///////////////////////////////////////// AABB /////////////////////////////////////////

AABB AABB::operator*(const Affine3f& transform) const {

	/// Theory on: http://dev.theomader.com/transform-bounding-boxes/

	Matrix4f matrix = transform.matrix();

	Vector3f min = center - half_extents;
	Vector3f max = center + half_extents;

	Vector3f min_transformed = Math::ToVector3(matrix.col(3));
	Vector3f max_transformed = Math::ToVector3(matrix.col(3));

	Vector3f a, b, col;

	for (int i = 0; i < 3; i++) {

		col = Math::ToVector3(matrix.col(i));

		a = col * min(i);
		b = col * max(i);

		min_transformed += Math::Min(a, b);
		max_transformed += Math::Max(a, b);

	}

	return AABB{ 0.5f * (max_transformed + min_transformed),
				 0.5f * (max_transformed - min_transformed) };

}

IntersectionType AABB::Intersect(const AABB& other) const {

	float diff[3];

	if ((diff[0] = std::fabs(other.center(0) - center(0)) - half_extents(0)) >= other.half_extents(0) ||
		(diff[1] = std::fabs(other.center(1) - center(1)) - half_extents(1)) >= other.half_extents(1) ||
		(diff[2] = std::fabs(other.center(2) - center(2)) - half_extents(2)) >= other.half_extents(2)) {

		return IntersectionType::kNone;		// Strictly separated

	}

	if (diff[0] < -other.half_extents(0) &&
		diff[1] < -other.half_extents(1) &&
		diff[2] < -other.half_extents(2)) {

		return IntersectionType::kInside | IntersectionType::kIntersect;	// Strictly enclosed (count as intersection)

	}

	return IntersectionType::kIntersect;	// Intersection

}

IntersectionType AABB::Intersect(const Sphere& sphere) const {

	// Clamps the sphere's center inside the box

	auto clamped_center = sphere.center.cwiseMax(center - half_extents).cwiseMin(center + half_extents);

	// If the distance between the clamped center and the actual sphere center is less than the sphere's radius, the two must intersect

	auto squared_sphere_radius = sphere.radius * sphere.radius;

	if ((sphere.center - clamped_center).squaredNorm() < squared_sphere_radius) {

		// Check whether the opposite corner of the box wrt the sphere center is inside the sphere.
		// The box is symmetrical: if the opposite corner is inside the sphere, every other corner is inside as well.

		auto max_corner = center + half_extents;
		auto min_corner = center - half_extents;

		Vector3f opposite_corner( center(0) > sphere.center(0) ? max_corner(0) : min_corner(0),
								  center(1) > sphere.center(1) ? max_corner(1) : min_corner(1),
								  center(2) > sphere.center(2) ? max_corner(2) : min_corner(2) );

		if ((sphere.center - opposite_corner).squaredNorm() < squared_sphere_radius) {

			return IntersectionType::kInside | IntersectionType::kIntersect;			// Count as intersection.

		}
		
		return IntersectionType::kIntersect;

	}
	else {

		return IntersectionType::kNone;

	}

}

///////////////////////////////////////// SPHERE /////////////////////////////////////////

Sphere Sphere::FromAABB(const AABB& aabb) {

	return Sphere{ aabb.center ,
				   aabb.half_extents.norm() };	// Implies a square root.

}

IntersectionType Sphere::Intersect(const Sphere& sphere) const {

	auto squared_distance = (sphere.center - center).squaredNorm();

	auto square_radius_this = radius * radius;
	auto square_radius_other = sphere.radius * sphere.radius;

	if (squared_distance <= square_radius_other + square_radius_this) {

		if (squared_distance <= square_radius_other - square_radius_this) {

			return IntersectionType::kInside | IntersectionType::kIntersect; // Count as intersection

		}

		return IntersectionType::kIntersect;

	}
	else {

		return IntersectionType::kNone;

	}

}

IntersectionType Sphere::Intersect(const AABB& box) const {

	// Clamps the sphere's center inside the box
	auto clamped_center = center.cwiseMax(box.center - box.half_extents).cwiseMin(box.center + box.half_extents);
	
	// If the distance between the clamped center and the actual sphere center is less than the sphere's radius, the two must intersect

	if( (center - clamped_center).squaredNorm() < radius * radius ){

		// If the sphere is contained wrt each axis, the sphere is totally contained inside the box

		auto axis_diff = (center - box.center).cwiseAbs() - box.half_extents;

		if (axis_diff(0) + radius <= 0 &&
			axis_diff(1) + radius <= 0 &&
			axis_diff(2) + radius <= 0 ) {

			return IntersectionType::kIntersect | IntersectionType::kInside;		// Count as intersection

		}

		return IntersectionType::kIntersect;

	}
	else {

		return IntersectionType::kNone;

	}
	
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