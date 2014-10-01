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
	
}