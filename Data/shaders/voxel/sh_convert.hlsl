/// \brief Used to convert the 3x int-encoded SH structure to a RGB float3-encoded one

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

cbuffer SHFilter {

	uint gDestinationCascade;						// Cascade of the destination surface where the data will be stored.

	uint gSourceCascade;							// Cascade of the source surface hwere the data will be read from.

	int gDestinationOffset;							// Offset applied to the destination pixels.

	int gDestinationMIP;							// MIP level of the destination surface.

};

#ifdef SH_STACK

Texture3D<int> gUnfilteredSHStack;								// Stack part of the unfiltered SH 3D clipmap.
RWTexture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

#define gSource gUnfilteredSHStack
#define gDestination gFilteredSHStack

#endif

#ifdef SH_PYRAMID

Texture3D<int> gUnfilteredSHPyramid;							// Pyramid part of the unfiltered SH 3D clipmap.
RWTexture3D<float3> gFilteredSHPyramid;							// Pyramid part of the filtered SH 3D clipmap.

#define gSource gUnfilteredSHPyramid
#define gDestination gFilteredSHPyramid

#endif

/// \brief Gather the int-encoded SH coefficients and write them as RGB float sample inside the given destination.
void GatherCoefficients(uint3 thread_id, uint coefficient_index, int cascade_index) {

	int resolution = gVoxelResolution >> gDestinationMIP;

	uint3 address = thread_id + uint3(coefficient_index, cascade_index, 0) * resolution;

	gDestination[address] = ToFloatSH(int3(gSource[address + uint3(0, 0, 0 * resolution)],
										   gSource[address + uint3(0, 0, 1 * resolution)],
										   gSource[address + uint3(0, 0, 2 * resolution)]));
	
}

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	uint band_count;
	
#ifdef SH_STACK

	uint cascades = gCascades;

#endif

#ifdef SH_PYRAMID

	uint cascades = 1;

#endif


	for (int cascade_index = 0; cascade_index < cascades; ++cascade_index) {

		band_count = GetSHBandCount(cascade_index);

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			GatherCoefficients(dispatch_thread_id, coefficient_index, cascade_index);
			
		}
		
	}
	
}