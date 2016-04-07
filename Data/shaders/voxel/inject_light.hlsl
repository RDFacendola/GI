/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#include "..\projection_def.hlsl"
#include "voxel_def.hlsl"

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D gRSM;		// Reflective shadow map. Contains the albedo of the surface and its normal.
Texture2D gVSM;		// Variance shadow map. Contains the first and the second moment of the depth distribution of the scene as seen from the light.

RWTexture3D<int> gVoxelSH;		// Voxel's spherical harmonics infos.



[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
		
	//float4 albedo = gRSM.Load(int3(dispatch_thread_id.xy, 0));
	//float depth = gVSM.Load(int3(dispatch_thread_id.xy, 0)).x;

	//// Using the light position, the uv coordinate and the depth compute the world position of the sample

	//uint2 dimensions;
	//uint dummy;

	//gVSM.GetDimensions(0, dimensions.x, dimensions.y, dummy);

	//float4 position_ps = TextureSpaceToProjectionSpace(dispatch_thread_id.xy / (float2)(dimensions), 
	//												   depth).xyz;

	//float3 position_ls = UnprojectFromOctahedronSpace(position_ps,
	//												  gNearPlane,
	//												  gFarPlane,
	//												  uv.x > 0.5f);			// Position of the fragment from the light's perspective
	//	
	//// To trasform to light-view space

	//float3 gLightCenter = float3(0, 200, 0);

	//float3 position_ws = position_ls + gLightCenter;

	//// Using the world position of the sample, determine which voxel the sample falls in

	//uint3 sh_address;
	//
	//if (WorldSpaceToSHAddress(position_ws, sh_address)) {

	//	// Using the direction of the light wrt the voxel center, determine the BRDF result and project the outgoing radiance into SH coefficient

	//	[unroll]
	//	for (int channel_index = 0; channel_index < 3; ++channel_index) {



	//	}

	//	float4 red_sh = float4(albedo.r * 1.0f, 0, 0, 0);
	//	float4 green_sh = float4(albedo.g * 1.0f, 0, 0, 0);
	//	float4 blue_sh = float4(albedo.b * 1.0f, 0, 0, 0);

	//	// Atomically update the SH inside the voxel

	//	uint mutex;

	//	[allow_uav_condition]
	//	do {

	//		InterlockedCompareExchange(gMutex[0], 0, 1, mutex);

	//		if (mutex == 0) {

	//			gRSH01[position_sh] += red_sh;
	//			gGSH01[position_sh] += green_sh;
	//			gBSH01[position_sh] += blue_sh;

	//			InterlockedCompareExchange(gMutex[0], 1, 0, mutex);

	//		}

	//	} while (mutex);

	//}


}