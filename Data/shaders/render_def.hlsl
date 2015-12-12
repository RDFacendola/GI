/// \file render_def.hlsl
/// \brief This file contains the definition of rendering structures
/// \author Raffaele D. Facendola

#ifndef RENDER_DEF_HLSL_
#define RENDER_DEF_HLSL_

#include "projection_def.hlsl"

/// GBuffer definition
struct GBuffer {

	float4 albedo : SV_Target0;							// Diffuse.R | Diffuse.G | Diffuse.B | Diffuse.A
	float4 normal_specular_shininess : SV_Target1;		// NormalWS.X | NormalWS.Y | Specular | Shininess

};

// GBuffer surfaces
Texture2D gAlbedo;						// Albedo.R | Albedo.G | Albedo.B | Albedo.A
Texture2D gNormalSpecularShininess;		// NormalWS.X | NormalWS.Y | Specular | Shininess
Texture2D gDepthStencil;				// Depth | Stencil

/// Surface infos
struct SurfaceData {

	float3 position;		// Position of the surface in world space.
	float4 albedo;			// Albedo of the surface.
	float3 normal;			// Normal of the surface in world space.
	float specular;			// Specular power.
	float shininess;		// Shininess for specular factor.

};

float3 ComputeSurfacePosition(float2 uv, float depth, float4x4 inv_view_proj_matrix) {

	return Unproject(inv_view_proj_matrix,
					 TextureSpaceToProjectionSpace(uv, depth)).xyz;

}

/// \see http://jcgt.org/published/0003/02/01/paper.pdf
float2 OctWrap(float2 v){

	return (1.0 - abs(v.yx)) * (v.xy >= 0.f ? 1.f : -1.f);

}

/// \brief Encodes a 3-element vector to a 2-element octahedron-encoded vector.
float2 EncodeNormals(float3 decoded) {

	decoded /= (abs(decoded.x) + abs(decoded.y) + abs(decoded.z));

	decoded.xy = (decoded.z >= 0.f) ? decoded.xy : OctWrap(decoded.xy);

	decoded.xy = decoded.xy * 0.5f + 0.5f;

	return decoded.xy;

}

/// \brief Decodes a 2-element octahedron-encoded vector to a 3-element vector.
float3 DecodeNormals(float2 encoded) {

	encoded = encoded * 2.f - 1.f;

	float3 decoded;

	decoded.z = 1.0 - abs(encoded.x) - abs(encoded.y);

	decoded.xy = (decoded.z >= 0.f) ? encoded.xy : OctWrap(encoded.xy);
	
	return normalize(decoded);
	
}

/// Gathers surface data from a sampling position inside the GBuffer surfaces
/// \param position Position inside the viewport (in pixels)
/// \param inv_view_proj_matrix Inverse view-projection matrix, used to pass from the projection space to the world space
SurfaceData GatherSurfaceData(uint2 position, float4x4 inv_view_proj_matrix) {

	// Sample the raw surface infos from the GBuffer

	float4 albedo = gAlbedo[position];
	float4 normal_specular_shininess = gNormalSpecularShininess[position];
	float depth = gDepthStencil[position].x;

	// Convert viewport coordinates to uv-like coordinates.

	float2 dimensions;

	gAlbedo.GetDimensions(dimensions.x, dimensions.y);

	float2 uv = position / dimensions;

	// Store the surface data

	SurfaceData surface_data;

	surface_data.position = ComputeSurfacePosition(uv, depth, inv_view_proj_matrix);
	surface_data.albedo = albedo;
	surface_data.normal = DecodeNormals(normal_specular_shininess.xy);
	surface_data.specular = normal_specular_shininess.z;
	surface_data.shininess = normal_specular_shininess.w;

	return surface_data;

}

#endif