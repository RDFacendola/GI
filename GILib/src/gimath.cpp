#include "..\include\gimath.h"

#include <math.h>

using namespace ::gi_lib;
using namespace ::Eigen;

float Math::kPi = 3.1415926f;

float Math::kDegToRad = Math::kPi / 180.0f;

float Math::kRadToDeg = 1.0f / Math::kDegToRad;

namespace {

	Vector3f ToVector3f(const Vector4f & vector){

		return Vector3f(vector(0), vector(1), vector(2));

	}

}

Bounds Bounds::Transformed(const Affine3f & transform) const{

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

	return Bounds{ 0.5f * (max_transformed + min_transformed),
		0.5f * (max_transformed - min_transformed) };
		
}

bool Bounds::Inside(const Bounds & other) const{

	return std::abs(other.center(0) - center(0)) < other.half_extents(0) - half_extents(0) &&
		std::abs(other.center(1) - center(1)) < other.half_extents(1) - half_extents(1) &&
		std::abs(other.center(2) - center(2)) < other.half_extents(2) - half_extents(2);

}

//////////////////////////// FRUSTUM /////////////////////////////////////

bool Frustum::Intersect(const Bounds & bounds) const{

	/// Theory on: http://www.gamedev.net/page/resources/_/technical/general-programming/useless-snippet-2-aabbfrustum-test-r3342

	// Test against every plane in the frustum
	for (unsigned int i = 0; i < 6; ++i)
	{

		const Vector3f & plane = ToVector3f(planes[i]); //The 4th component (ie. the distance) is not needed for the distance and the radius.

		float distance = bounds.center.dot(plane);
						 
		float radius = bounds.half_extents.dot(plane.cwiseAbs());

		if (distance + radius < -planes[i](3)){

			return false;

		}

	}

	return true;

}