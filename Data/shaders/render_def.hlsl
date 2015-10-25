/// \file render_def.hlsl
/// \brief This file contains the definition of rendering structures
/// \author Raffaele D. Facendola

#ifndef RENDER_DEF_HLSL_
#define RENDER_DEF_HLSL_

/// GBuffer definition
struct GBuffer {

	float4 albedo : SV_Target0;					// Diffuse.R | Diffuse.G | Diffuse.B | Diffuse.A
	float4 normal_shininess : SV_Target1;		// NormalWS.X | NormalWS.Y | NormalWS.Z | Shininess

};

// GBuffer surfaces
Texture2D gAlbedo;				// Albedo.R | Albedo.G | Albedo.B | Albedo.A
Texture2D gNormalShininess;		// NormalWS.X | NormalWS.Y | NormalWS.Z | Shininess
Texture2D gDepthStencil;		// Depth | Stencil

/// Surface infos
struct SurfaceData {

	float3 position;		// Position of the surface in world space.
	float4 albedo;			// Albedo of the surface.
	float3 normal;			// Normal of the surface in world space.
	float shininess;		// Shininess for specular factor.

};

float3 ComputeSurfacePosition(float2 uv, float depth, float4x4 inv_view_proj_matrix) {

	// Compute the position in projection space

	float4 position_ps = float4(uv * float2(2.0, -2.0) + float2(-1.0f, 1.0f),		// [-1;1] - [1;-1] (left;top)-(right;bottom)
							    depth, 
							    1.0f);

	// Unproject the position into world space

	float4 position_ws = mul(inv_view_proj_matrix, position_ps);

	position_ws /= position_ws.w;

	return position_ws.xyz;

}

/// Gathers surface data from a sampling position inside the GBuffer surfaces
/// \param position Position inside the viewport (in pixels)
/// \param inv_view_proj_matrix Inverse view-projection matrix, used to pass from the projection space to the world space
SurfaceData GatherSurfaceData(uint2 position, float4x4 inv_view_proj_matrix) {

	// Sample the raw surface infos from the GBuffer

	float4 albedo = gAlbedo[position];
	float4 normal_shininess = gNormalShininess[position];
	float depth = gDepthStencil[position].x;

	// Convert viewport coordinates to uv-like coordinates.

	float2 dimensions;

	gAlbedo.GetDimensions(dimensions.x, dimensions.y);

	float2 uv = position / dimensions;

	// Store the surface data

	SurfaceData surface_data;

	surface_data.position = ComputeSurfacePosition(uv, depth, inv_view_proj_matrix);
	surface_data.albedo = albedo;
	surface_data.normal = normalize(normal_shininess.xyz);
	surface_data.shininess = normal_shininess.w;

	return surface_data;

}

#endif