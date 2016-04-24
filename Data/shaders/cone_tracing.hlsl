#include "render_def.hlsl"
#include "voxel\voxel_def.hlsl"

#define N 16
#define TOTAL_THREADS (N)

StructuredBuffer<uint> gVoxelAddressTable;					// Contains the "pointers" to the actual voxel infos.
Texture3D<int> gVoxelSH;									// Contains SH infos

RWTexture2D<float4> gLightAccumulation;						// Indirect light accumulation buffer

cbuffer gParameters {

	float4x4 inv_view_proj_matrix;		///< \brief Inverse view-projection matrix.
	float4 camera_position;				///< \brief Camera position in world space.
	uint point_lights;					///< \brief Number of point lights.
	uint directional_lights;			///< \brief Number of directional lights.

};

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	SurfaceData surface = GatherSurfaceData(dispatch_thread_id.xy, inv_view_proj_matrix);

	VoxelInfo voxel_info;

	if (!GetVoxelInfo(gVoxelAddressTable, surface.position, voxel_info)) {

		return;

	}

	float4 ic = SampleVoxelColor(gVoxelSH, voxel_info, surface.normal);

	// Sum the indirect contribution inside the light accumulation buffer

	gLightAccumulation[dispatch_thread_id.xy] += float4(surface.albedo * ic.rgb, 1);

}