/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#define N 16
#define TOTAL_THREADS (N * N)

Texture2D gRSM;		// Reflective shadow map. Contains the albedo of the surface and its normal.
Texture2D gVSM;		// Variance shadow map. Contains the first and the second moment of the depth distribution of the scene as seen from the light.

RWTexture3D<float4> gRSH01;		// First and second SH coefficients for the red channel
RWTexture3D<float4> gGSH01;		// First and second SH coefficients for the green channel
RWTexture3D<float4> gBSH01;		// First and second SH coefficients for the blue channel

[numthreads(N, N, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	float4 p = gRSM.Load(int3(dispatch_thread_id.xy, 0));
	float2 q = gVSM.Load(int3(dispatch_thread_id.xy, 0)).xy;

	// Using the light position, the uv coordinate and the depth compute the world position of the sample

	// Using the world position of the sample, determine which voxel the sample falls in

	// Using the direction of the light wrt the voxel center, determine the BRDF result and project the outgoing radiance into SH coefficient

	// Atomically update the SH inside the voxel

	// Profit!

	gRSH01[dispatch_thread_id.xyz] = float4(1, 1, 0, 0);
	gGSH01[dispatch_thread_id.xyz] = float4(0, 0, 2, 0);
	gBSH01[dispatch_thread_id.xyz] = float4(0, 0, 1.5f, 3);

}