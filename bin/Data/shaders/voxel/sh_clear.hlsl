
/// \brief Clears the spherical harmonic structure

#define N 8
#define TOTAL_THREADS (N * N * N)

#include "voxel_def.hlsl"

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	gSHRed[dispatch_thread_id] = 0;
	gSHGreen[dispatch_thread_id] = 0;
	gSHBlue[dispatch_thread_id] = 0;
	gSHAlpha[dispatch_thread_id] = 0;

}