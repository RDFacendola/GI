/// \brief Used to inject lighting inside the spherical harmonics structure starting from a RSM

#include "voxel_def.hlsl"

#define N 8
#define TOTAL_THREADS (N * N * N)

Texture3D<int> gUnfilteredSHPyramid;							// Pyramid part of the unfiltered SH 3D clipmap.
Texture3D<int> gUnfilteredSHStack;								// Stack part of the unfiltered SH 3D clipmap.

RWTexture3D<float3> gFilteredSHPyramid;							// Pyramid part of the filtered SH 3D clipmap.
RWTexture3D<float3> gFilteredSHStack;							// Stack part of the filtered SH 3D clipmap.

float3 LoadCoefficients(Texture3D<int> sh, uint3 thread_id, uint coefficient_index, int cascade_index) {

	uint3 base_offset = uint3(coefficient_index, 
							  (uint)cascade_index, 
							  0) * gVoxelResolution;

	uint3 r_offset = base_offset + uint3(0, 0, 0) * gVoxelResolution;
	uint3 g_offset = base_offset + uint3(0, 0, 1) * gVoxelResolution;
	uint3 b_offset = base_offset + uint3(0, 0, 2) * gVoxelResolution;

	// Gather from different Z channels inside a single float3

	return ToFloatSH(int3(sh[thread_id + r_offset],
						  sh[thread_id + g_offset],
						  sh[thread_id + b_offset]));
	
}

void StackFilter(uint3 thread_id, uint coefficient_index, int cascade_index) {

	// Filter inside the stack

	uint3 address = thread_id + uint3(coefficient_index, cascade_index, 0) * gVoxelResolution;

	gFilteredSHStack[address] = LoadCoefficients(gUnfilteredSHStack, thread_id, coefficient_index, cascade_index);


}

void PyramidFilter(uint3 thread_id, uint coefficient_index, uint mip_index) {

	// Each thread samples a 2^3 area and sums the various coefficients

	uint3 sample_location;

	sample_location.xyz = thread_id * 2;

	sample_location.x += coefficient_index * gVoxelResolution;

	// Gather contributions from the previous MIP level

	//sample_location.w = mip_index - 1;

	//float3 coefficients = gFilteredSHPyramid.Load(sample_location);

	// Store the contribution inside the current MIP level

	uint3 address = thread_id + uint3(coefficient_index, 0, 0) * gVoxelResolution;

	gFilteredSHPyramid[address] = LoadCoefficients(gUnfilteredSHPyramid, thread_id, coefficient_index, 0);
	
}

[numthreads(N, N, N)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	uint band_count;

	// Stack filtering

	for (int cascade_index = (int)gCascades; cascade_index >= 0; --cascade_index) {

		band_count = GetSHBandCount(cascade_index);	// Some SH contribution are lost if the number of bands does not match

		for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

			StackFilter(dispatch_thread_id, coefficient_index, cascade_index);
			
		}
		
	}
	
	// Pyramid filtering

	band_count = GetSHBandCount(0);

	for (uint coefficient_index = 0; coefficient_index < band_count * band_count; ++coefficient_index) {

		PyramidFilter(dispatch_thread_id, coefficient_index, 0);

	}
	
}