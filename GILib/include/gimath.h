/// \file gimath.h
/// \brief Mathematical and geometrical classes and methods.
///
/// \author Raffaele D. Facendola

#pragma once

#if _MSC_VER >= 1500

#pragma warning(push)
#pragma warning(disable:4127)

#include <Eigen\Geometry>

#pragma warning(pop)

#else

#include <Eigen\Geometry>

#endif

#include <math.h>
#include <algorithm>

using ::Eigen::Vector3f;
using ::Eigen::Vector4f;
using ::Eigen::Affine3f;

namespace gi_lib{

	/// \brief Wraps common math functions.
	/// \author Raffaele D. Facendola
	class Math{

	public:

		static float kRadToDeg;

		static float kDegToRad;
			
		static float kPi;

		/// \brief Convert radians to degrees.
		/// \param radians Radians.
		/// \return Return the angle in degrees.
		static float RadToDeg(float radians);

		/// \brief Convert degrees to radians.
		/// \param degrees Degrees.
		/// \return Return the angle in radians.
		static float DegToRad(float degrees);

		/// \brief Check whether two numbers are essentially equal.

		/// \param a The first number to test.
		/// \param b The second number to test.
		/// \param epsilon The maximum error percentage. Defines the error range around the smallest number between a and b.
		/// \return Returns true if the bigger number falls within the error range of the smallest one. Returns false otherwise.
		static bool Equal(float a, float b, float epsilon);

		/// \brief Finds the component-wise minimum of two 3-dimensional vectors.
		/// \param left First operand.
		/// \param right Second operand.
		static Vector3f Min(const Vector3f & left, const Vector3f & right);

		/// \brief Finds the component-wise maximum of two 3-dimensional vectors.
		/// \param left First operand.
		/// \param right Second operand.
		static Vector3f Max(const Vector3f & left, const Vector3f & right);

	};

	/// \brief Represents the bound of a geometry
	struct Bounds{

		/// \brief Center of the bounds.
		Vector3f center;

		/// \brief Half-extents of the bounds (ie: Width x Height x Depth)
		Vector3f half_extents;

		/// \brief Transform the bounding box using an affine transformation matrix.
		/// \param transform Matrix used to transform the bounding box.
		/// \return Returns a new bounding box which is the transformed version of this instance.
		Bounds Transformed(const Affine3f & transform) const;

		/// \brief Check whether this bounds are strictly inside the specified ones.
		/// \param other The bounds to check inclusion against.
		/// \return Returns true if the bounds are strictly contained inside 'other', false otherwise.
		bool Inside(const Bounds & other) const;

	};

	/// \brief Frustum represented by 6 planes
	struct Frustum{

		/// \brief Planes composing the frustum.

		/// The order of the planes is not defined.
		Vector4f planes[6];

		/// \brief Check whether the frustum contains or intersect a given aabb.
		/// \param bounds The bounds to test against.
		bool Intersect(const Bounds & bounds) const;

	};

	// math

	inline float Math::RadToDeg(float radians){

		return radians * kRadToDeg;

	}

	inline float Math::DegToRad(float degrees){

		return degrees * kDegToRad;

	}

	inline bool Math::Equal(float a, float b, float epsilon)
	{
		
		/// From "The art of computer programming by Knuth"
		return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);

	}


	inline Vector3f Math::Min(const Vector3f & left, const Vector3f & right){

		return Vector3f(std::min<float>(left(0), right(0)),
			std::min<float>(left(1), right(1)),
			std::min<float>(left(2), right(2)));

	}

	inline Vector3f Math::Max(const Vector3f & left, const Vector3f & right){
		
		return Vector3f(std::max<float>(left(0), right(0)),
			std::max<float>(left(1), right(1)),
			std::max<float>(left(2), right(2)));


	}
		
}