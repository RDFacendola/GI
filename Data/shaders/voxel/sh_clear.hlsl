/// \brief Clears the spherical harmonic structure

#define N 8
#define TOTAL_THREADS (N * N * N)

RWTexture3D<float4> gRSH01;		// First and second SH coefficients for the red channel
RWTexture3D<float4> gGSH01;		// First and second SH coefficients for the green channel
RWTexture3D<float4> gBSH01;		// First and second SH coefficients for the blue channel

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	gRSH01[dispatch_thread_id.xyz] = 0.0f;
	gGSH01[dispatch_thread_id.xyz] = 0.0f;
	gBSH01[dispatch_thread_id.xyz] = 0.0f;

}