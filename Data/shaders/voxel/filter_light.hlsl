/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

StructuredBuffer<uint> gVoxelAddressTable;						// Contains the "pointers" to the actual voxel infos.
RWTexture3D<int> gVoxelSH;										// Voxel's spherical harmonics infos.

int SampleSHArea(uint3 dispatch_thread_id, uint coefficient, uint cascade, uint channel) {

	uint3 thread = (dispatch_thread_id * 2) + uint3(coefficient * gVoxelResolution, 
												    cascade * gVoxelResolution, 
												    channel * gVoxelResolution);

	return gVoxelSH[thread + uint3(0,0,0)] +
		   gVoxelSH[thread + uint3(0,0,1)] +
		   gVoxelSH[thread + uint3(0,1,0)] +
		   gVoxelSH[thread + uint3(0,1,1)] +
		   gVoxelSH[thread + uint3(1,0,0)] +
		   gVoxelSH[thread + uint3(1,0,1)] +
		   gVoxelSH[thread + uint3(1,1,0)] +
		   gVoxelSH[thread + uint3(1,1,1)];

}

void Filter(uint3 dispatch_thread_id, uint coefficient, uint cascade, uint channel) {

	uint3 thread = dispatch_thread_id + (gVoxelResolution >> 2) + uint3(coefficient * gVoxelResolution,
																	    (cascade - 1) * gVoxelResolution, 
																	    channel * gVoxelResolution);

	// No other thread will stomp on this location, no need to resort to atomics

	gVoxelSH[thread] = SampleSHArea(dispatch_thread_id, 
									coefficient, 
									cascade, 
									channel);

}

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	// Each thread accumulate the light from 2x2x2 area

	// Spherical harmonics are linear, meaning that the sum of different SH is also the sum of their coefficients

	for (uint cascade_index = gCascades - 1; cascade_index > 0; --cascade_index) {

		uint coefficient_count = GetSHBandCount(cascade_index);	

		coefficient_count *= coefficient_count;		// The target cascade may have a higher number of bands, we cannot reconstruct those!

		[flatten]
		for (uint coefficient_index = 0; coefficient_index < coefficient_count; ++coefficient_index) {

			Filter(dispatch_thread_id, coefficient_index, cascade_index, 0);	// Red
			Filter(dispatch_thread_id, coefficient_index, cascade_index, 1);	// Green
			Filter(dispatch_thread_id, coefficient_index, cascade_index, 2);	// Blue
			
		}
		
	}
	
}