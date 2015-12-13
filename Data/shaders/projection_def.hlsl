/// \file projection_def.hlsl
/// \brief This file contains methods used to compute projections.
/// \author Raffaele D. Facendola

#ifndef PROJECTION_DEF_HLSL_
#define PROJECTION_DEF_HLSL_

/// \brief Convert a coordinate from projection space to texture space.
/// \param position_ps Coordinates in projection space.
/// \return Returns the associated coordinates in uv space.
float2 ProjectionSpaceToTextureSpace(float4 position_ps) {

	return float2(position_ps.x * 0.5f + 0.5f,
				  0.5f - position_ps.y * 0.5f);

}

/// \brief Convert a coordinate from texture space to projection space.
/// \param position_ts Coordinates in texture space. [0; 1] x [0; 1]
/// \param depth Depth of the projected point.
/// \return Returns the associated coordinates in projection space.
float4 TextureSpaceToProjectionSpace(float2 position_ts, float depth) {

	return float4(position_ts * float2(2.0, -2.0) + float2(-1.0f, 1.0f),
				  depth,
				  1.0f);

}

/// \brief Unproject a coordinate from projection space to another space.
/// \param space Matrix associated to the destination space.
float4 Unproject(float4x4 space, float4 position_ps) {

	float4 position_ws = mul(space, position_ps);

	position_ws /= position_ws.w;

	return position_ws;

}

/// \brief Project a point into octahedron space.
/// \param position Coordinates of the point to project.
/// \param near_clipping Near clipping plane.
/// \param far_plane Far clipping plane.
float4 ProjectToOctahedronSpace(float3 position, float near_plane, float far_plane) {

	float depth = length(position);														// Depth of the point wrt the center of the octahedron projection (0;0;0)

	depth = saturate((length(position) - near_plane) / (far_plane - near_plane));		// Fragments closer than the near plane are fully lit, while fragments beyond the far plane are fully shadowed.

	float3 I = position / (abs(position.x) + abs(position.y) + abs(position.z));		// Normalize using Manhattan distance
	
	// Rotate I by 45 degrees and scale up to fill the entire space

	float cos_theta = 0.7071f;															// cos(45deg) = sin(45deg)
	
	float2 J = float2(I.x * cos_theta - I.y * cos_theta,
					  I.x * cos_theta + I.y * cos_theta) * 1.4142f;						// Scale up to fill the entire square

	return float4(J.x,
				  J.y,
				  depth,
				  1.0f);

}

/// \brief Project a point into paraboloid space.
/// \param position Coordinates of the point to project.
/// \param near_clipping Near clipping plane.
/// \param far_plane Far clipping plane.
float4 ProjectToParaboloidSpace(float3 position, float near_plane, float far_plane) {

	// The paraboloid is of the form 
	// f(x,y) = 0.5 - 0.5(x^2 + y^2)
	
	// T = d f(x,y) / dx = (1, 0, -x)					// tangent
	// B = d f(x,y) / dy = (0, 1, -y)					// bitangent
	// N = T x B = (x, y, 1)							// normal (*)

	// (*) tells us that knowing N, we know the projected position Q of the point on the paraboloid surface

	// Incident rays I = P/|P| are reflected along the direction R = (0, 0, 1) due to the paraboloid properties

	// R = I - 2*N*dot(N,I) =>							// Reflection R from incident I and normal N
	// R + I || N =>
	// (Ix + 0, Iy + 0, Iz + 1) || (Qx, Qy, 1) =>		// Normalize on the left
	
	// | Qx = Ix / Iz + 1
	// | Qy = Iy / Iz + 1
	// | Qz = 1

	float depth = length(position);					// Depth of the point wrt the center of the paraboloid projection (0;0;0)

	float3 I = position / depth;					

	return float4(I.x / (abs(I.z) + 1),
				  I.y / (abs(I.z) + 1),
				  saturate((depth - near_plane) / (far_plane - near_plane)),		// Fragments closer than the near plane are fully lit, while fragments beyond the far plane are fully shadowed.
				  1.0f);

}

#endif