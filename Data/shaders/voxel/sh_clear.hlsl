/// \brief Clears the spherical harmonic structure

#define N 8
#define TOTAL_THREADS (N * N * N)

RWTexture3D<int> gVoxelSH;		// Contains SH infos

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	gVoxelSH[dispatch_thread_id] = 0;

}