/// \file render_def.hlsl
/// \brief This file contains the definition of rendering structures
/// \author Raffaele D. Facendola

#ifndef RENDER_DEF_HLSL_
#define RENDER_DEF_HLSL_

#define PI 3.14159f

#include "projection_def.hlsl"
#include "encode_def.hlsl"

/// GBuffer definition
struct GBuffer {

	float4 albedo_emissivity : SV_Target0;				// Diffuse.R | Diffuse.G | Diffuse.B | Emissivity
	float4 normal_specular_shininess : SV_Target1;		// NormalWS.X | NormalWS.Y | Specular | Shininess

};

// GBuffer surfaces
Texture2D gAlbedoEmissivity;							// Albedo.R | Albedo.G | Albedo.B | Emissivity
Texture2D gNormalSpecularShininess;						// NormalWS.X | NormalWS.Y | Specular | Shininess
Texture2D gDepthStencil;								// Depth | Stencil

/// Surface infos
struct SurfaceData {

	float3 position;		// Position of the surface in world space.
	
	float3 albedo;			// Albedo of the surface.
	float3 normal;			// Normal of the surface in world space.

	//float specular;			// Specular power.
	//float shininess;		// Shininess for specular factor.

	float emissivity;		// Emissivity of the surface (self-illumination).
	float roughness;		// Roughness of the material. Physically based.
	float metalness;		// Metalness of the material. Physically based.
	float cavity;			// Small scale occlusion, as multiplicative factor over the specular term. Somewhat physically based.

	float2 ior;				// Index of refraction of the material. The second element is the absorption. Physically based.
	float kd;				// Percentage of diffused radiance. [0;1]. Note that energy conservation requires kd + ks <= 1.
	float ks;				// Percentage of reflected radiance. [0;1]. Note that energy conservation requires kd + ks <= 1.

};

float3 ComputeSurfacePosition(float2 uv, float depth, float4x4 inv_view_proj_matrix) {

	return Unproject(inv_view_proj_matrix,
					 TextureSpaceToProjectionSpace(uv, depth)).xyz;

}

/// Gathers surface data from a sampling position inside the GBuffer surfaces
/// \param position Position inside the viewport (in pixels)
/// \param inv_view_proj_matrix Inverse view-projection matrix, used to pass from the projection space to the world space
SurfaceData GatherSurfaceData(uint2 position, float4x4 inv_view_proj_matrix) {

	// Sample the raw surface infos from the GBuffer

	float4 albedo_emissivity = gAlbedoEmissivity[position];
	float4 normal_specular_shininess = gNormalSpecularShininess[position];
	float depth = gDepthStencil[position].x;

	// Convert viewport coordinates to uv-like coordinates.

	float2 dimensions;

	gAlbedoEmissivity.GetDimensions(dimensions.x, dimensions.y);

	float2 uv = position / dimensions;

	// Store the surface data

	SurfaceData surface_data;

	surface_data.position = ComputeSurfacePosition(uv, depth, inv_view_proj_matrix);
	surface_data.albedo = albedo_emissivity.xyz;
	surface_data.normal = DecodeNormals(normal_specular_shininess.xy);

	//surface_data.specular = normal_specular_shininess.z;
	//surface_data.shininess = normal_specular_shininess.w;

	// TODO: plug the actual values of the surface

	surface_data.emissivity = albedo_emissivity.w;
	surface_data.roughness = 0.4;
	surface_data.metalness = 0;
	surface_data.cavity = normal_specular_shininess.z;

	surface_data.ior = float2(1.3, 0);
	surface_data.kd = 0.6f;
	surface_data.ks = 0.4f;

	return surface_data;

}

#endif