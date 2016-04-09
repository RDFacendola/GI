/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#include "..\projection_def.hlsl"
#include "voxel_def.hlsl"

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D gRSM;		// Reflective shadow map. Contains the albedo of the surface and its normal.
Texture2D gVSM;		// Variance shadow map. Contains the first and the second moment of the depth distribution of the scene as seen from the light.

StructuredBuffer<uint> gVoxelAddressTable;						// Contains the "pointers" to the actual voxel infos.
RWTexture3D<int> gVoxelSH;										// Voxel's spherical harmonics infos.

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
		
	float4 surface_albedo = gRSM.Load(int3(dispatch_thread_id.xy, 0));
	float depth = gVSM.Load(int3(dispatch_thread_id.xy, 0)).x;

	// Unproject the sample position to world space

	uint2 dimensions;
	uint dummy;

	gVSM.GetDimensions(0, dimensions.x, dimensions.y, dummy);

	// Projection space

	float4 position_ps = TextureSpaceToProjectionSpace(dispatch_thread_id.xy / (float2)(dimensions), 
													   depth);

	// Light-view space

	float3 position_ls = UnprojectFromOctahedronSpace(position_ps,
													  gNearPlane,
													  gFarPlane,
													  position_ps.x > 0.f);
	
	// World space

	float3 position_ws = mul(gLightMatrix, float4(position_ls, 1)).xyz;

	// Get the current voxel

	VoxelInfo voxel_info;

	GetVoxelInfo(gVoxelAddressTable, position_ws, voxel_info);	// We skip the check since the voxel *should* exist

	// Compute photon contribution, project into SH coefficients and such

	float3 sh_coefficients[4];		// Each float represents the coefficient of each of the 3 color channels.

	sh_coefficients[0] = float3(0.05f, 0.05f, 0.05f) * surface_albedo.rgb;
	sh_coefficients[1] = float3(0, 0, 0) * surface_albedo.rgb;
	sh_coefficients[2] = float3(0, 0, 0) * surface_albedo.rgb;
	sh_coefficients[3] = float3(0, 0, 0) * surface_albedo.rgb;

	// Store the SH contribution

	StoreSHCoefficients(gVoxelSH, voxel_info, sh_coefficients);
	
}