/// \brief Used to convert the 3x int-encoded SH structure to a RGB float3-encoded one

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

Texture3D<int> gUnfilteredSHStack;								// Stack part of the unfiltered SH 3D clipmap.
RWTexture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

Texture3D<int> gUnfilteredSHPyramid;							// Pyramid part of the unfiltered SH 3D clipmap.
RWTexture3D<float3> gFilteredSHPyramid;							// Pyramid part of the filtered SH 3D clipmap.

/// \brief Gather the int-encoded SH coefficients and write them as RGB float sample inside the given destination.
void GatherCoefficients(Texture3D<int> source, RWTexture3D<float3> destination, uint3 thread_id, uint coefficient_index, int cascade_index) {

	uint3 address = thread_id + uint3(coefficient_index, cascade_index, 0) * gVoxelResolution;

	destination[address] = ToFloatSH(int3(source[address + uint3(0, 0, 0 * gVoxelResolution)],
										  source[address + uint3(0, 0, 1 * gVoxelResolution)],
										  source[address + uint3(0, 0, 2 * gVoxelResolution)]));
	
}

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	uint band_count;

	// Stack gathering

	for (int cascade_index = 0; cascade_index < gCascades; ++cascade_index) {

		band_count = GetSHBandCount(cascade_index);

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			GatherCoefficients(gUnfilteredSHStack, gFilteredSHStack, dispatch_thread_id, coefficient_index, cascade_index);
			
		}
		
	}
	
	// Pyramid gathering - What about the MIPS?

	band_count = GetSHBandCount(0);

	for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

		GatherCoefficients(gUnfilteredSHPyramid, gFilteredSHPyramid, dispatch_thread_id, coefficient_index, 0);

	}
	
}