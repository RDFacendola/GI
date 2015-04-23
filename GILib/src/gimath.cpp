#include "..\include\gimath.h"

#include <math.h>

using namespace ::gi_lib;
using namespace ::Eigen;

float Math::kPi = 3.1415926f;

float Math::kDegToRad = Math::kPi / 180.0f;

float Math::kRadToDeg = 1.0f / Math::kDegToRad;

namespace {

	Vector4f ToHomogeneous(const Vector3f & vector){

		return Vector4f(vector(0), vector(1), vector(2), 1.0f);

	}

	Vector3f ToVector3f(const Vector4f & vector){

		return Vector3f(vector(0), vector(1), vector(2));

	}

}

///////////////////////////////////////// AABB /////////////////////////////////////////

AABB AABB::operator*(const Affine3f& transform) const{

	/// Theory on: http://dev.theomader.com/transform-bounding-boxes/

	Matrix4f matrix = transform.matrix();

	Vector3f min = center - half_extents;
	Vector3f max = center + half_extents;

	Vector3f min_transformed = ToVector3f(matrix.col(3));
	Vector3f max_transformed = ToVector3f(matrix.col(3));

	Vector3f a, b, col;

	for (int i = 0; i < 3; i++){

		col = ToVector3f(matrix.col(i));

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

IntersectionType Frustum::Intersect(const AABB& bounds) const{

	/// Theory on: https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/

	int i = 0;

	auto hcenter = ToHomogeneous(bounds.center);	// Needed to compute the distance as a dot product between two 4-elements vectors.

	for (auto& plane : planes){

		if (plane.dot(hcenter) < -bounds.half_extents.dot(abs_normals[i])){

			return IntersectionType::kNone;			// Outside the plane

		}
		
		++i;

	}

	return IntersectionType::kIntersect;	// Intersection

}

IntersectionType Frustum::Intersect(const Sphere& sphere) const{

	auto hcenter = ToHomogeneous(sphere.center);	// Needed to compute the distance as a dot product between two 4-elements vectors.

	for (auto& plane : planes){

		if (plane.dot(hcenter) < -sphere.radius ){

			return IntersectionType::kNone;			// Outside the plane

		}

	}

	return IntersectionType::kIntersect;

}