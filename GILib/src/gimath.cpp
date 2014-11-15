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

Bounds Bounds::Transformed(const Affine3f & transform){

	/// Theory on: http://dev.theomader.com/transform-bounding-boxes/

	Matrix4f matrix = transform.matrix();

	Vector3f half = extents * 0.5f;

	Vector3f min = center - half;
	Vector3f max = center + half;

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
		max_transformed - min_transformed };
		
}

bool Bounds::Inside(const Bounds & other){

	return 2.0f * std::abs(other.center(0) - center(0)) < other.extents(0) - extents(0) &&
		2.0f * std::abs(other.center(1) - center(1)) < other.extents(1) - extents(1) &&
		2.0f * std::abs(other.center(2) - center(2)) < other.extents(2) - extents(2);

}