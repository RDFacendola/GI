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

	uint sh_bands;						// Number of bands used by the voxel.

	uint3 padding;

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

/// \brief Converts a floating point SH coefficient to an integer number.
int ToIntSHCoefficient(float sh_coefficient) {

	return sh_coefficient * 100;

}

/// \brief Converts an integer SH coefficient to a float number.
float ToFloatSHCoefficient(int sh_coefficient) {

	return sh_coefficient / 100.f;

}

/// \brief Create a voxel.
/// \param voxel_address_table Structure containing the pointers to the voxelized scene.
/// \param voxel_coordinates Coordinates of the voxel to create relative to its own cascade.
/// \param cascade Index of the cascade the voxel belongs to.
void Voxelize(RWStructuredBuffer<uint> voxel_address_table, uint3 voxel_coordinates, uint cascade) {
		
	uint linear_coordinates = voxel_coordinates.x +
							  voxel_coordinates.y * gVoxelResolution +
							  voxel_coordinates.z * gVoxelResolution * gVoxelResolution;

	uint linear_address = linear_coordinates + GetClipmapPyramidSize() + GetCascadeSize() * cascade;

	// WORKAROUND (***) - Fill with the actual address!
	
	voxel_address_table[linear_address] = 42;

}

/// \brief Get the voxel info given a voxel pointer.
/// \param voxel_address_table Linear array containing the voxel pointer structure.
/// \param index Index of the pointer pointing to the voxel whose infos are requested.
/// \param voxel_info If the method succeeds it contains the requested voxel informations.
/// \return Returns true if the specified pointer was valid, returns false otherwise.
bool GetVoxelInfo(StructuredBuffer<uint> voxel_address_table, uint index, out VoxelInfo voxel_info) {

	// WORKAROUND (***) - Get and translate the actual address
	// TODO: manage the case where the index is negative!

	if (index < GetClipmapPyramidSize() ||
		voxel_address_table[index] == 0) {

		return false;

	}

	index -= GetClipmapPyramidSize();

	uint3 voxel_ptr3 = uint3(index,
						     index / gVoxelResolution,
						     index / (gVoxelResolution * gVoxelResolution)) % gVoxelResolution;
	
	// Fill out the voxel info
	
	voxel_info.cascade = index / GetCascadeSize();
	
	voxel_info.size = gVoxelSize / pow(2, voxel_info.cascade);
	
	voxel_info.center = (((int3)(voxel_ptr3) - (int)(gVoxelResolution >> 1)) + 0.5f) * voxel_info.size + gCenter;

	voxel_info.sh_address = voxel_ptr3 % gVoxelResolution;

	voxel_info.sh_bands = 2;		// 2 bands, for now

	voxel_info.padding = 0;

	return true;

}

/// World space to voxel info (Inject)
bool GetVoxelInfo(float3 position_ws, out VoxelInfo voxel_info) {

	// World space -> VAT -> voxel info
	return false;

}

/// Interlocked add of SH coefficients
void StoreSHCoefficients(RWTexture3D<int> voxel_sh, VoxelInfo voxel_info, float3 sh_coefficients) {


}

/// \brief Get the SH coefficients values of the given voxel.
/// \param voxel_info Voxel to sample.
/// \param sh_index Index of the spherical harmonics coefficients.
/// \return Return the value of the specified spherical harmonic for each color channel.
float4 GetSHCoefficients(Texture3D<int> voxel_sh, VoxelInfo voxel_info, uint sh_index) {

	// ISSUE: If we keep the SH coefficients together we have better cache locality but we lose hardware filtering among coefficients of adjacent voxels 
	//		  If we spread out the SH coefficients we have hardware filtering of adjacent voxels but we lose cache locality.

	// IDEA:
	// We may store the texture as Texture3D<int4> for reading purposes but bind it with an UAV like RWTexture3D<int>.
	// We have hardware filtering which is even faster as it operates on 4 channels at the same time while mantaining cache locality among them
	// We waste one int. Maybe we can recycle it for the opacity?

	// int are 32 bits which is way too much for a single SH coefficient. We may pack 2 coefficients for each int giving us the possibility of storing 8 of them.
	// - R0 16 | R1 16 | G0 16 | G1 16 | B0 16 | B1 16 | <wasted> 32 ?

	float4 coefficients;

	uint3 sh_address = voxel_info.sh_address;

	sh_address.x += sh_index * gVoxelResolution;					// Move to the correct SH
	sh_address.y += voxel_info.cascade * gVoxelResolution;			// Move to the correct cascade
							
	coefficients.r = ToFloatSHCoefficient(voxel_sh[sh_address]);	// Move to the red channel
		
	sh_address.z += gVoxelResolution;								// Move to the green channel
	coefficients.g = ToFloatSHCoefficient(voxel_sh[sh_address]);

	sh_address.z += gVoxelResolution;								// Move to the blue channel
	coefficients.b = ToFloatSHCoefficient(voxel_sh[sh_address]);

	// Done
	
	coefficients.a = 0.f;

	return coefficients;

}

/// \brief Get the SH contribution in a given direction for a particular SH band.
/// \param voxel_sh Structure containing the SH coefficients.
/// \param voxel_info Voxel to sample.
/// \param sh_band Index of the SH band to calculate the contribution of.
/// \param direction Direction of the sampling.
float4 GetSHContribution(Texture3D<int> voxel_sh, VoxelInfo voxel_info, uint sh_band, float3 direction) {

	if (sh_band == 0) {

		return GetSHCoefficients(voxel_sh, voxel_info, 0) * sqrt(1.f / (4.f * PI));

	}
	else if (sh_band == 1) {

		return (GetSHCoefficients(voxel_sh, voxel_info, 1) * direction.x +
			    GetSHCoefficients(voxel_sh, voxel_info, 2) * direction.y +
			    GetSHCoefficients(voxel_sh, voxel_info, 3) * direction.z) * sqrt(3.f / (4.f * PI));

	}
	else {

		return 0;

	}

}

/// \brief Sample the color of a voxel in the given direction.
/// \param voxel_sh Structure containing the SH coefficients.
/// \param voxel_info Voxel to sample.
/// \param direction Direction of the sampling.
float4 SampleVoxelColor(Texture3D<int> voxel_sh, VoxelInfo voxel_info, float3 direction) {

	float4 color = 0.f;

	for (uint sh_band = 0; sh_band < voxel_info.sh_bands; ++sh_band) {

		color += GetSHContribution(voxel_sh, voxel_info, sh_band, direction);

	}

	return float4(color.rgb, 1.0f);

}

#endif