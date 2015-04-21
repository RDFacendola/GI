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

#include "enums.h"

using ::Eigen::Vector2f;
using ::Eigen::Vector3f;
using ::Eigen::Vector4f;
using ::Eigen::Affine3f;

namespace gi_lib{

	/// \brief Intersection types.
	ENUM_FLAGS(IntersectionType, unsigned int){

		kSeparate,		///< \brief Separate objects.
		kOverlapping,	///< \brief Overlapping objects
		kInside,		///< \brief The first object is completely inside the second object
		kOutside,		///< \brief The first object is completely outside the second object
		
	};

	/// \brief Axis-aligned bounding box.
	struct AABB{

		Vector3f center;		///< \brief Center of the bounds.

		Vector3f half_extents;  ///< \brief Half-extents of the bounds in each direction.

		/// \brief Transform the bounding box using an affine transformation matrix.
		/// \param transform Matrix used to transform the bounding box.
		/// \return Returns a new bounding box which is the transformed version of this instance.
		AABB operator*(const Affine3f& transform) const;

		/// \brief Check whether this bounds are strictly inside the specified ones.
		/// \param other The bounds to check inclusion against.
		/// \return Returns true if the bounds are strictly contained inside 'other', false otherwise.
		bool Inside(const AABB& other) const;

	};

	/// \brief Bounding sphere.
	struct Sphere{

		Vector3f center;	///< \brief Center of the sphere.

		float radius;		///< \brief Radius of the sphere.

		/// \brief Approximate the specified box with a sphere.
		/// \param aabb Box to approximate
		/// \return Returns a sphere which is an approximation of the specified box.
		static Sphere FromAABB(const AABB& aabb);

		/// \brief Approximate the specified box with a sphere with a squared radius.
		/// \param aabb Box to approximate
		/// \return Returns a sphere which is an approximation of the specified box but with a squared radius.
		static Sphere FromAABBSquared(const AABB& aabb);

	};

	/// \brief Frustum represented by 6 planes
	struct Frustum{

		/// \brief Planes composing the frustum.

		/// The order of the planes is not defined.
		Vector4f planes[6];

		/// \brief Check whether the frustum contains or intersect a given aabb.
		/// \param aabb The AABB to test against.
		bool Intersect(const AABB& aabb) const;

	};
	
	/// \brief Wraps common math functions.
	/// \author Raffaele D. Facendola
	class Math{

	public:

		/// \brief Factor used to convert a radian to a degree.
		static float kRadToDeg;

		/// \brief Factor used to convert a degree to a radian.
		static float kDegToRad;
			
		/// \brief Pi.
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

	//////////////////////////////// MATH ////////////////////////

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