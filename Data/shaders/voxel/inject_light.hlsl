/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#include "..\encode_def.hlsl"
#include "..\projection_def.hlsl"
#include "..\light_def.hlsl"

#include "voxel_def.hlsl"

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D gRSM;		// Reflective shadow map. Contains the albedo of the surface and its normal.
Texture2D gVSM;		// Variance shadow map. Contains the first and the second moment of the depth distribution of the scene as seen from the light.

StructuredBuffer<uint> gVoxelAddressTable;						// Contains the "pointers" to the actual voxel infos.
RWTexture3D<int> gVoxelSH;										// Voxel's spherical harmonics infos.

cbuffer CBPointLight {

	PointLight gPointLight;			// Point light injecting the photons inside the voxel structure.

};

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
		
	float4 fragment_albedo = gRSM.Load(int3(dispatch_thread_id.xy, 0));
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

	if (!GetVoxelInfo(gVoxelAddressTable, position_ws, voxel_info)) {

		return;

	}

	// The shadowmaps contains all and only the fragments that receive energy from the pointlight. 
	// Since the fragments are projected in light-view space, we don't need attenuation from depth as it is compensated by the smaller projected angle.

	// Amount of energy received by the voxel

	float3 voxel_flux = gPointLight.color.rgb * rcp(1000.f);		// voxel_flux = gPointLight.color.rgb * rcp(dimensions.x * dimensions.y) -> this leads to a massive loss of precision!

	float3 fragment_normals = DecodeNormalsCoarse(fragment_albedo.w);

	// Compute photon contribution, project into SH coefficients

	float3 sh_coefficients[4];		// Each float represents the coefficient of each of the 3 color channels.

	//voxel_flux *= fragment_albedo.rgb * (4.f * 3.1416f);
	voxel_flux *= fragment_albedo.rgb * 15.f;

	sh_coefficients[0] = float3(1, 1, 1) * voxel_flux;
	sh_coefficients[1] = float3(0, 0, 0) * voxel_flux;
	sh_coefficients[2] = float3(0, 0, 0) * voxel_flux;
	sh_coefficients[3] = float3(0, 0, 0) * voxel_flux;

	// Store the SH contribution

	StoreSHCoefficients(gVoxelSH, voxel_info, sh_coefficients);
	
}