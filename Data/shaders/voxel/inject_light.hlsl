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

	// energy = gPointLight.color.rgb * rcp(dimensions.x * dimensions.y) -> this leads to a massive loss of precision!
	float3 energy = gPointLight.color.rgb * rcp(100.f);

	// Project energy into SH coefficients.

	float3 sh_coefficients[4];		

	float3 fragment_normal = DecodeNormalsCoarse(fragment_albedo.w);

	// Light is diffused in all direction around 90 degrees from surface normal.

	float3 diffuse_energy = energy * fragment_albedo.rgb;

	// Each float represents the coefficient of each of the 3 color channels.

	sh_coefficients[0] = sqrt(1.f / (4.f * PI)) * diffuse_energy;
	sh_coefficients[1] = fragment_normal.x * diffuse_energy;
	sh_coefficients[2] = fragment_normal.y * diffuse_energy;
	sh_coefficients[3] = fragment_normal.z * diffuse_energy;

	// Store the SH contribution

	StoreSHCoefficients(gVoxelSH, voxel_info, sh_coefficients);
	
}