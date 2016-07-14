#include "render_def.hlsl"
#include "voxel\voxel_def.hlsl"
#include "pbr_def.hlsl"

#define N 16
#define TOTAL_THREADS (N)

StructuredBuffer<uint> gVoxelAddressTable;					// Contains the "pointers" to the actual voxel infos.

RWTexture2D<float3> gIndirectLight;

cbuffer gParameters {

	float4x4 inv_view_proj_matrix;		///< \brief Inverse view-projection matrix.
	float4 camera_position;				///< \brief Camera position in world space.
	uint point_lights;					///< \brief Number of point lights.
	uint directional_lights;			///< \brief Number of directional lights.

};

/// Gathers surface data from a sampling position inside a downscaled GBuffer surfaces.
/// The sampled GBuffer is a quarter of the original resolution
/// \param position Position inside the viewport (in pixels)
/// \param inv_view_proj_matrix Inverse view-projection matrix, used to pass from the projection space to the world space
SurfaceData SampleSurfaceData4(uint2 position, float4x4 inv_view_proj_matrix) {

	// Convert viewport coordinates to uv-like coordinates.

	float2 dimensions;

	gAlbedoEmissivity.GetDimensions(dimensions.x, dimensions.y);

	float2 uv = position / dimensions;

	// Sample the raw surface infos from the GBuffer

	float4 albedo_emissivity = gAlbedoEmissivity.SampleLevel(gSHSampler, uv, 0);
	float4 normal_specular_shininess = gNormalSpecularShininess.SampleLevel(gSHSampler, uv, 0);
	
	float4 depths = float4(gDepthStencil[position + int2(0, 0)].x,
						   gDepthStencil[position + int2(0, 1)].x,
						   gDepthStencil[position + int2(1, 0)].x,
						   gDepthStencil[position + int2(1, 1)].x);

	float depth = min(depths.x, min(depths.y, min(depths.z, depths.w)));

	// Store the surface data

	SurfaceData surface_data;

	surface_data.position = ComputeSurfacePosition(uv, depth, inv_view_proj_matrix);
	surface_data.albedo = albedo_emissivity.xyz;
	surface_data.normal = DecodeNormals(normal_specular_shininess.xy);

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

float3 SampleSpecularCone(SurfaceData surface, float3 light_direction, float3 view_direction) {

	float4 color = SampleCone(surface.position,
							  light_direction,
							  0.15f,
							  15);
	
	float3 specular = ComputeCookTorrance(light_direction, view_direction, surface);	// Cook-Torrance specular (PBR).

	return surface.ks * specular * color.rgb * saturate(1.0f - surface.emissivity);

}

float3 SampleDiffuseCone(SurfaceData surface, float3 light_direction, float3 view_direction) {

	float4 color = SampleCone(surface.position,
							  light_direction,
							  0.20f,
							  5);

	float3 diffuse = ComputeLambert(light_direction, view_direction, surface);			// Lambertian diffuse

	return surface.kd * diffuse * color.rgb * saturate(1.0f - surface.emissivity);

}

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	SurfaceData surface = SampleSurfaceData4(dispatch_thread_id.xy * 2 + uint2(1, 1), inv_view_proj_matrix);
	
	float3 V = normalize(camera_position.xyz - surface.position);	// From the camera to the surface

	float3 R = reflect(-V,
					   surface.normal);

	float3 color = 0;

	color += SampleSpecularCone(surface, R, V) * 0.2;									// Specular

	float3 x = cross(surface.normal, normalize(float3(1,1,1)));
	float3 y = cross(surface.normal, x);

	x = cross(surface.normal, y);

	x *= 3.f;
	y *= 3.f;

	//color += SampleDiffuseCone(surface, surface.normal, V) * 0.1f;						// Diffuse Up
	//color += SampleDiffuseCone(surface, normalize(surface.normal + x), V) * 0.1f;
	//color += SampleDiffuseCone(surface, normalize(surface.normal - x), V) * 0.1f;
	//color += SampleDiffuseCone(surface, normalize(surface.normal + y), V) * 0.1f;
	//color += SampleDiffuseCone(surface, normalize(surface.normal - y), V) * 0.1f;

	// Sum the indirect contribution inside the light accumulation buffer
	
    gIndirectLight[dispatch_thread_id.xy] = max(0, 1.0f * float4(color, 1));

}