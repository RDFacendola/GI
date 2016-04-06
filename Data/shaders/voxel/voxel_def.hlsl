/// \file voxel_def.hlsl
/// \brief This file contains shared definition for voxels
///
/// \author Raffaele D. Facendola

#ifndef VOXEL_VOXEL_DEF_HLSL_
#define VOXEL_VOXEL_DEF_HLSL_

#define PI 3.14159f

cbuffer Voxelization {

	float3 gCenter;						// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;					// Size of each voxel in world units for each dimension.

	uint gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	uint gCascades;						// Number of additional cascades inside the clipmap.

};

/// \brief Hold a single voxel's data
struct VoxelInfo {

	float3 center;						// Center of the voxel, in world space

	float size;							// Size of the voxel in world units

	float4 red_sh01;					// First and second SH coefficients for the red channel.

	float4 green_sh01;					// First and second SH coefficients for the green channel.

	float4 blue_sh01;					// First and second SH coefficients for the blue channel.

};

/// \brief Converts a linear address to 3D voxel address.
uint3 LinearAddressTo3D(uint address) {

	uint3 coordinates = uint3(address,
							  address / gVoxelResolution,
							  address / (gVoxelResolution * gVoxelResolution));

	return coordinates % gVoxelResolution;

}

bool WorldSpaceToSHAddress(float3 position, out uint3 sh_address) {

	// Find which cascade the point falls in

	position -= gCenter;		// Position of the fragment from the voxelization center's perspective.

	//float max_distance = max(position.x, max(position.y, position.z));

	//int cascade = max(0, gCascades - max_distance / (gVoxelSize * 0.5f * pow(2.0f, -(int)(gCascades))));

	//float voxel_size = gVoxelSize * pow(2.0f, -cascade);

	int3 voxel_position = position / gVoxelSize;	// [-Resolution/2; Resolution/2]

	voxel_position += (gVoxelResolution >> 1);		// [0; Resolution]

	sh_address = uint3(voxel_position.x,
					   voxel_position.y,
					   voxel_position.z);

	return true;
/*
	return abs(voxel_position.x) < (int)(gVoxelResolution) &&
		   abs(voxel_position.y) < (int)(gVoxelResolution) &&
		   abs(voxel_position.z) < (int)(gVoxelResolution);
	*/	
}

/// \brief Sample a spherical harmonic
float SampleSH(float4 sh01, float3 direction){

	// see http://mathworld.wolfram.com/SphericalHarmonic.html
	//		https://ssl.cs.dartmouth.edu/~wjarosz/publications/dissertation/appendixB.pdf

	float value = 0.f;

	// SH0

	value += sh01.x * sqrt(1.f / (4.f * PI));

	// SH1

	value += sh01.y * sqrt(3.f / (4.f * PI)) * direction.x;

	value += sh01.z * sqrt(3.f / (4.f * PI)) * direction.y;

	value += sh01.w * sqrt(3.f / (4.f * PI)) * direction.z;

	return value;

}

float4 SampleSH(VoxelInfo info, float3 direction) {

	return float4(SampleSH(info.red_sh01, direction),
			      SampleSH(info.green_sh01, direction),
				  SampleSH(info.blue_sh01, direction),
				  0.75f);

}

#endif