
/// \brief Clears the spherical harmonic structure

#define N 8
#define TOTAL_THREADS (N * N * N)

RWTexture3D<int> gUnfilteredSHPyramid;								// Pyramid part of the unfiltered SH 3D clipmap.
RWTexture3D<int> gUnfilteredSHStack;								// Stack part of the unfiltered SH 3D clipmap.

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	// Stack

	gUnfilteredSHStack[dispatch_thread_id] = 0;

	// Pyramid

	gUnfilteredSHPyramid[dispatch_thread_id] = 0;

}