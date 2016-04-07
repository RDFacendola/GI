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

	uint3 sh_address;					// Address of the SH coefficients.

	int cascade;						// Cascade the voxel falls in. A negative number indicates a MIP map.

};

/// \brief Get the total amount of voxels inside a cascade
uint GetCascadeSize() {

	return gVoxelResolution * gVoxelResolution * gVoxelResolution;

}

/// \brief Get the total amount of voxels inside the pyramid of the voxel clipmap, without the last level.
uint GetClipmapPyramidSize() {

	// Simplified form of a sum of a geometric series S = (1 - 8^log2(gVoxelResolution)) / (1 - 8)

	return (GetCascadeSize() - 1) / 7;

}

/// Create a new empty voxel
void Voxelize(RWStructuredBuffer<uint> voxel_address_table, uint3 voxel_coordinates, uint cascade) {
		
	uint linear_coordinates = voxel_coordinates.x +
							  voxel_coordinates.y * gVoxelResolution +
							  voxel_coordinates.z * gVoxelResolution * gVoxelResolution;

	uint linear_address = linear_coordinates + GetClipmapPyramidSize() + GetCascadeSize() * cascade;

	// WORKAROUND (***) - Fill with the actual address!
	
	voxel_address_table[linear_address] = 42;

}

/// VAT address to voxel info (Append)
bool GetVoxelInfo(StructuredBuffer<uint> voxel_address_table, uint index, out VoxelInfo voxel_info) {

	// WORKAROUND (***) - Get and translate the actual address

	int resolution = gVoxelResolution;

	if (index < GetClipmapPyramidSize() ||
		voxel_address_table[index] == 0) {

		return false;

	}

	// TODO: manage the case where the index is negative!

	index -= GetClipmapPyramidSize();

	int3 voxel_ptr3 = int3(index,
						   index / resolution,
						   index / (resolution * resolution)) % resolution;
	
	// Fill out the voxel info

	voxel_info.cascade = index / GetCascadeSize();
	
	voxel_info.size = gVoxelSize / pow(2, voxel_info.cascade);
	
	voxel_info.center = ((voxel_ptr3 - (resolution >> 1)) + 0.5f) * voxel_info.size + gCenter;

	voxel_info.sh_address = voxel_ptr3 % resolution;

	return true;

}

/// World space to voxel info (Inject)
bool GetVoxelInfo(float3 position_ws, out VoxelInfo voxel_info) {

	// World space -> VAT -> voxel info
	return false;

}

/// Voxel info to voxel color (Outline)
bool SampleVoxelColor(RWTexture3D<int> voxel_sh, VoxelInfo voxel_info, float3 direction, out float4 color) {

	return false;

}

/// Interlocked add of SH coefficients
void StoreSHCoefficients(RWTexture3D<int> voxel_sh, VoxelInfo voxel_info, float3 sh_coefficients) {


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
	//	   https://ssl.cs.dartmouth.edu/~wjarosz/publications/dissertation/appendixB.pdf

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

	//return float4(SampleSH(info.red_sh01, direction),
	//		      SampleSH(info.green_sh01, direction),
	//			  SampleSH(info.blue_sh01, direction),
	//			  0.75f);

	return 0;

}

#endif