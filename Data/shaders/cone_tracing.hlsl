#include "render_def.hlsl"
#include "voxel\voxel_def.hlsl"

#define N 16
#define TOTAL_THREADS (N)

StructuredBuffer<uint> gVoxelAddressTable;					// Contains the "pointers" to the actual voxel infos.

Texture3D<float3> gFilteredSHPyramid;						// Pyramid part of the filtered SH 3D clipmap.
Texture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

RWTexture2D<float4> gLightAccumulation;

cbuffer gParameters {

	float4x4 inv_view_proj_matrix;		///< \brief Inverse view-projection matrix.
	float4 camera_position;				///< \brief Camera position in world space.
	uint point_lights;					///< \brief Number of point lights.
	uint directional_lights;			///< \brief Number of directional lights.

};

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	SurfaceData surface = GatherSurfaceData(dispatch_thread_id.xy, inv_view_proj_matrix);
	
	float3 reflection_direction = reflect(normalize(camera_position.xyz - surface.position), 
										  surface.normal);

	float3 color = 0;

	for (int step = 0; step < 10; ++step) {

		color += SampleVoxelColor(gFilteredSHPyramid, gFilteredSHStack, surface.position - reflection_direction * step * 100.f, reflection_direction);

	}

	// Sum the indirect contribution inside the light accumulation buffer

	gLightAccumulation[dispatch_thread_id.xy] += float4(color.rgb, 1);

}