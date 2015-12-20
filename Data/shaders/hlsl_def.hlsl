/// \file hlsl_def.hlsl
/// \brief This file contains general HLSL methods.
/// \author Raffaele D. Facendola

#ifndef HLSL_DEF_HLSL_
#define HLSL_DEF_HLSL_

/// Get the sign of the specified value as bit mask
/// 100: negative
/// 010: zero
/// 001: positive
/// The method sets only one bit
int GetSignMask(float value) {

	return (1 << (1 - sign(value))) & 0x7;

}

/// Get the composite sign mask of the specified values
/// 1XX: at least one value was negative
/// X1X: at least one value was zero
/// XX1: at least one value was positive
int GetSignMask(float a, float b, float c) {

	return GetSignMask(a) | GetSignMask(b) | GetSignMask(c);

}

float4 GetIntersection(float4 a, float4 b, float dota, float dotb) {

	return lerp(a,
				b,
				-dota / (dotb - dota));

}

/// \brief Slices a triangle given a splitting plane.
/// \param polygon Triangle to split.
/// \param plane The plane the triangle will be splitted against.
/// \param strip Resulting triangle strip.
/// \return Returns true if the polygon was splitted, returns false otherwise.
bool SlicePolygon(float4 polygon[3], float4 plane, out float4 strip[5]) {

	// TODO: Can we reduce the branchiness from here?

	float dots[3];

	dots[0] = dot(polygon[0], plane);
	dots[1] = dot(polygon[1], plane);
	dots[2] = dot(polygon[2], plane);

	int dot01 = sign(dots[0] * dots[1]);
	int dot02 = sign(dots[0] * dots[2]);
	int dot12 = sign(dots[1] * dots[2]);

	bool sliced = true;

	if (!(dot01 < 0 || dot02 < 0 || dot12 < 0)) {

		strip[0] = polygon[0];
		strip[1] = polygon[1];
		strip[2] = polygon[2];
		strip[3] = polygon[1];		// Not really needed
		strip[4] = polygon[2];		// Not really needed

		sliced = false;				// The polygon should not be split!

	}
	else if (dot01 < 0 && dot12 >= 0) {

		strip[0] = polygon[0];
		strip[1] = GetIntersection(polygon[0], polygon[1], dots[0], dots[1]);
		strip[2] = GetIntersection(polygon[0], polygon[2], dots[0], dots[2]);
		strip[3] = polygon[1];
		strip[4] = polygon[2];

	}
	else if (dot12 < 0 && dot02 >= 0) {

		strip[0] = polygon[1];
		strip[1] = GetIntersection(polygon[1], polygon[2], dots[1], dots[2]);
		strip[2] = GetIntersection(polygon[1], polygon[0], dots[1], dots[0]);
		strip[3] = polygon[2];
		strip[4] = polygon[0];

	}
	else {

		strip[0] = polygon[2];
		strip[1] = GetIntersection(polygon[2], polygon[0], dots[2], dots[0]);
		strip[2] = GetIntersection(polygon[2], polygon[1], dots[2], dots[1]);
		strip[3] = polygon[0];
		strip[4] = polygon[1];

	}

	return sliced;

}

#endif