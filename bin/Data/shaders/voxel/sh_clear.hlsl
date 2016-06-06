
/// \brief Clears the spherical harmonic structure

#define N 8
#define TOTAL_THREADS (N * N * N)

RWTexture3D<int> gSHRed;				// Unfiltered red spherical harmonics contributions.
RWTexture3D<int> gSHGreen;				// Unfiltered green spherical harmonics contributions.
RWTexture3D<int> gSHBlue;				// Unfiltered blue spherical harmonics contributions.

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {

	gSHRed[dispatch_thread_id] = 0;
	gSHGreen[dispatch_thread_id] = 0;
	gSHBlue[dispatch_thread_id] = 0;
	
}