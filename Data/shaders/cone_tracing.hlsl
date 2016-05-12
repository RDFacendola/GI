#include "render_def.hlsl"
#include "voxel\voxel_def.hlsl"
#include "pbr_def.hlsl"

#define N 16
#define TOTAL_THREADS (N)

StructuredBuffer<uint> gVoxelAddressTable;					// Contains the "pointers" to the actual voxel infos.

Texture3D<float3> gFilteredSHPyramid;						// Pyramid part of the filtered SH 3D clipmap.
Texture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

Texture2D<float4> gLightAccumulation;
RWTexture2D<float4> gIndirectLight;

cbuffer gParameters {

	float4x4 inv_view_proj_matrix;		///< \brief Inverse view-projection matrix.
	float4 camera_position;				///< \brief Camera position in world space.
	uint point_lights;					///< \brief Number of point lights.
	uint directional_lights;			///< \brief Number of directional lights.

};

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	SurfaceData surface = GatherSurfaceData(dispatch_thread_id.xy, inv_view_proj_matrix);
	
	float3 eye_direction = normalize(surface.position - camera_position.xyz);	// From the camera to the surface

	float3 reflection_direction = reflect(eye_direction,
										  surface.normal);

	float fresnel = 1.0f - saturate(dot(reflection_direction, surface.normal));

	fresnel = pow(fresnel, 3.0f);

	float3 color = 0;

	float3 ray = abs(reflection_direction);

	ray *= rcp(max(ray.x, max(ray.y, ray.z)));

	float step_size = length(ray) * ((int)(gVoxelSize) >> 0);
		
	float sample_distance;
	float3 sample_location;

	for (int step = 1; step < 10; ++step) {

		sample_distance = step * step_size;
		sample_location = surface.position + sample_distance * reflection_direction;

		color += SampleVoxelColor(gFilteredSHPyramid, 
								  gFilteredSHStack, 
								  sample_location,
								  -reflection_direction,
								  0);

	}

	color *= ComputeCookTorrance(reflection_direction, -eye_direction, surface) * 0.7f;
	
	// Sum the indirect contribution inside the light accumulation buffer

    gIndirectLight[dispatch_thread_id.xy] = gLightAccumulation[dispatch_thread_id.xy] + max(0, float4(color.rgb, 1));

}