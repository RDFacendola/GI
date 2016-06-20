/// \file voxel_def.hlsl
/// \brief This file contains shared definition for voxels
///
/// \author Raffaele D. Facendola

#ifndef VOXEL_VOXEL_DEF_HLSL_
#define VOXEL_VOXEL_DEF_HLSL_

#define PI 3.14159f

RWTexture3D<int> gSHRed;				// Unfiltered red spherical harmonics contributions.
RWTexture3D<int> gSHGreen;				// Unfiltered green spherical harmonics contributions.
RWTexture3D<int> gSHBlue;				// Unfiltered blue spherical harmonics contributions.

Texture3D<float4> gSH;					// Filtered chromatic spherical harmonics contribution.

SamplerState gSHSampler;				// Sampler used to sample the SH

cbuffer Voxelization {

	float3 gCameraCenter;				// Center of the voxelization. Each cascade may snap to a different position.

	float gVoxelSize;					// Size of each voxel in world units for each dimension.

	uint gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	uint gCascades;						// Number of additional cascades inside the clipmap.

};

/// \brief Hold a single voxel's data
struct VoxelInfo {

	float3 center;						// Center of the voxel, in world space

	float size;							// Size of the voxel in world units

	uint3 sh_address;					// Address of the SH coefficients. Without the cascade!

	int cascade;						// Cascade the voxel falls in. A negative number indicates a MIP map.

	uint sh_bands;						// Number of bands used by the voxel.

	uint3 padding;

};

///////////////////////////////////////////////////////////////////////
/* SHARED															 */
///////////////////////////////////////////////////////////////////////

/// \brief Converts a floating point color to a fixed point int-encoded one.
/// This method is needed since InterlockedAdd of floats is not supported.
int3 ToIntSH(float3 color) {

	return color * 50;

}

float4 ToFloatSH(int4 sh_coefficient) {

	return sh_coefficient * 0.001f;

}

/// \brief Get the voxel size of a given MIP level.
/// \param mip_index MIP level index.
/// \return Returns the size of the voxel of the given MIP level in world units.
float GetVoxelSize(int mip_index) {

	// return (gVoxelSize * (1 << (gCascades + mip_index))) / (1 << gCascades);

	return ldexp(gVoxelSize, mip_index);		// gVoxelSize * (2 ^ mip_index)

}

/// \brief Get the total amount of voxels inside a cascade
int GetCascadeSize() {

	return gVoxelResolution * gVoxelResolution * gVoxelResolution;

}

/// \brief Get the center of a MIP level giving its voxel size.
/// \param voxel_size Size of the voxel belonging to the MIP level.
/// \return Returns the center of the MIP level whose voxel size is the specified one.
float3 GetMIPCenter(float voxel_size) {

	return floor(gCameraCenter / voxel_size) * voxel_size;

}

/// \brief Get the total amount of voxels inside the pyramid of the voxel clipmap, without the last level.
int GetClipmapPyramidSize() {

	// Simplified form of a sum of a geometric series S = (1 - 8^log2(gVoxelResolution)) / (1 - 8)

	return (GetCascadeSize() - 1) / 7;

}

/// \brief Get the number of SH bands stored for each voxel in a specified cascade.
uint GetSHBandCount(uint cascade_index) {

	return 2;		// 2 SH bands, for now

}

/// \brief Get the Chebyshev distance between two 3D points.
/// \return Returns the Chebyshev distance between two 3D points.
float GetChebyshevDistance(float3 a, float3 b) {

	float3 ab = abs(a - b);

	return max(ab.x, max(ab.y, ab.z));

}

/// \brief Get the Chebyshev distance between two 1D points.
/// \return Returns the Chebyshev distance between two 1D points.
float GetChebyshevDistance(float a, float b) {

	return abs(a - b);

}

/// \brief Get the MIP level index of a point in world space.
/// \param position_ws World space position of the point.
/// \return Returns the index of MIP level containing the specified point.
/// \remarks Positive MIP values here mean that the point was outside the domain!
int GetMIPLevel(float3 position_ws) {

	// TODO: I'm pretty sure there's some formula that avoids the cycle below.
	// CRITICAL OPTIMIZATION: this function is called during light injection, filtering and light accumulation (per pixel).
	//						  It KILLS the performances!

	float half_resolution = gVoxelResolution >> 1;

	float3 mip_center;
	float voxel_size;
	float distance;

	int mip_level = 1;

	do {

		--mip_level;	// Evaluate next MIP level

		voxel_size = GetVoxelSize(mip_level);
		mip_center = GetMIPCenter(voxel_size);

		distance = GetChebyshevDistance(position_ws, mip_center) / voxel_size;

	} while (distance < half_resolution &&
		     mip_level >= -(int)gCascades);

	return mip_level + 1;
			
}

/// \brief Get the minimum size of a voxel around the specified point.
float GetMinVoxelSize(float3 position_ws) {

	return GetVoxelSize(GetMIPLevel(position_ws));

}

///////////////////////////////////////////////////////////////////////
/* VOXELIZATION 													 */
///////////////////////////////////////////////////////////////////////

/// \brief Create a voxel.
/// \param voxel_address_table Structure containing the pointers to the voxelized scene.
/// \param voxel_coordinates Coordinates of the voxel to create relative to its own cascade.
/// \param cascade Index of the cascade the voxel belongs to.
void Voxelize(RWStructuredBuffer<uint> voxel_address_table, uint3 voxel_coordinates, int cascade) {

	uint linear_coordinates = voxel_coordinates.x +
							  voxel_coordinates.y * gVoxelResolution +
							  voxel_coordinates.z * gVoxelResolution * gVoxelResolution;

	uint linear_address = linear_coordinates + GetClipmapPyramidSize() + GetCascadeSize() * -cascade;

	// WORKAROUND (***) - Fill with the actual address!

	voxel_address_table[linear_address] = 42;

}

/// \brief Get the voxel info given a voxel pointer.
/// \param voxel_address_table Linear array containing the voxel pointer structure.
/// \param index Index of the pointer pointing to the voxel whose infos are requested.
/// \param voxel_info If the method succeeds it contains the requested voxel informations.
/// \return Returns true if the specified pointer was valid, returns false otherwise.
bool GetVoxelInfo(StructuredBuffer<uint> voxel_address_table, int index, out VoxelInfo voxel_info) {

	// WORKAROUND (***) - Get and translate the actual address
	// TODO: manage the case where the index is negative!

	bool is_inside_clipmap = index >= (int)GetClipmapPyramidSize() && 
							 index < (int)(GetClipmapPyramidSize() + GetCascadeSize() * (1 + gCascades));

//#define DENSE_VOXEL_INFO

// Define this MACRO if access of the whole SH structure is needed. This doesn't work if the structure is sparse! Default is NOT DEFINED.
#ifndef DENSE_VOXEL_INFO

	is_inside_clipmap = is_inside_clipmap && (voxel_address_table[index] != 0);

#endif

	if (!is_inside_clipmap) {

		return false;		// The voxel is not present inside the hierarchy or it is outside its bounds

	}

	index -= GetClipmapPyramidSize();

	uint3 voxel_ptr3 = uint3(index,
						     index / gVoxelResolution,
						     index / (gVoxelResolution * gVoxelResolution)) % gVoxelResolution;
	
	// Fill out the voxel info

	voxel_info.cascade = -(index / GetCascadeSize());
	
	voxel_info.size = GetVoxelSize(voxel_info.cascade);
	
	voxel_info.center = (((int3)(voxel_ptr3) - (int)(gVoxelResolution >> 1)) + 0.5f) * voxel_info.size + GetMIPCenter(voxel_info.size);

	voxel_info.sh_address = voxel_ptr3 % gVoxelResolution;

	voxel_info.sh_bands = 2;		// 2 bands, for now

	voxel_info.padding = 0;

	return true;

}

///////////////////////////////////////////////////////////////////////
/* PHOTON INJECTION													 */
///////////////////////////////////////////////////////////////////////

/// \brief Store the chromatic spherical harmonic contribution for a given position in space.
/// \param position_ws Position of the contribution in world space.
/// \param sh_index Linear index of the spherical harmonic coefficient to store.
/// \param color Color contribution to store.
void StoreSHContribution(float3 position_ws, uint sh_index, float3 color) {

	// During light injection it is only possible to fill the MIP levels whose index is 0 or negative.
	// Having a positive MIP level here means that the sample is outside the voxel domain and should be discarded.
	
	int mip_index = GetMIPLevel(position_ws);

	if (mip_index <= 0) {

		float voxel_size = GetVoxelSize(mip_index);
		
		// Texture coordinates
	
		position_ws -= GetMIPCenter(voxel_size);
		
		int3 coords = floor(position_ws * rcp(voxel_size)) + (gVoxelResolution >> 1);

		// Offset - Move to the correct coefficient and MIP level
		// The top slice of the texture is skipped as it contains the MIP levels > 0

		coords += int3(sh_index, 1 - mip_index, 0) * gVoxelResolution;

		// InterlockedAdd of floats is not supported, convert to fixed-precision int

		int3 icolor = ToIntSH(color);

		// Store the color - SH are linear: the sum of different SHs is the sum of their coefficients.
	
		InterlockedAdd(gSHRed[coords], icolor.r);
		InterlockedAdd(gSHGreen[coords], icolor.g);
		InterlockedAdd(gSHBlue[coords], icolor.b);

	}

}

///////////////////////////////////////////////////////////////////////
/* SAMPLING															 */
///////////////////////////////////////////////////////////////////////

int GetSHCoordinates(float3 position_vs, uint coefficient_index, int cascade, out float3 address) {

	float voxel_size = GetVoxelSize(cascade);

	address = position_vs * rcp(voxel_size) + (gVoxelResolution * 0.5f);

	address.x += coefficient_index * gVoxelResolution;					// Move to the correct coefficient
	address.y += sign(cascade) * (cascade - 1) * gVoxelResolution;		// Move to the correct MIP. The sign(.) here is to bypass the instruction if we are already inside the pyramid.

	return sign(cascade) + 1;		// 0 for cascade < 0
									// 1 for cascade = 0
									// 2 for cascade > 0

}

float4 SampleSH(float3 position_ws, int sh_index, int mip_level) {

	float3 dimensions;

	gSH.GetDimensions(dimensions.x,									// gVoxelResolution * #Coefficients
					  dimensions.y,									// gVoxelResolution * (#Cascades + 2)
					  dimensions.z);								// gVoxelResolution

	int coefficients = dimensions.x / gVoxelResolution;				// Maximum number of SH coefficients

	float voxel_size = GetVoxelSize(mip_level);

	float3 position_vs = position_ws - GetMIPCenter(voxel_size);

	float scale = 1 << max(0, mip_level);

	float3 sample_location = ((scale * position_vs) / (voxel_size * gVoxelResolution)) + 0.5f;		// [0; 1]

	sample_location.x = (sample_location.x + sh_index) / (scale * coefficients);					// Move to the proper SH coefficient

	sample_location.y = (sample_location.y + 1 - min(mip_level, 0)) / (scale * (gCascades + 2));	// Move to the proper MIP level
	
	return gSH.SampleLevel(gSHSampler, sample_location, 0);											// Sample

}

float4 SampleSHCoefficients(float3 position_ws, uint sh_index, float radius) {
 
	// Assumption: radius is always at least the maximum resolution available for the specified sample location. Avoids multiple GetMinVoxelSize(.)

	float voxel_size = radius * 2.0f;

	// Clamp the maximum resolution to the maximum allowed for the specified level

	float mip_level = max(GetMIPLevel(position_ws),
						  -log2(gVoxelSize / voxel_size));

	float4 f_sample = SampleSH(position_ws, sh_index, floor(mip_level));
	float4 c_sample = SampleSH(position_ws, sh_index, ceil(mip_level));

	return lerp(c_sample, f_sample, mip_level - floor(mip_level));			// Quadrilinear interpolation between two successive MIP levels

}

float4 SampleSHContribution(float3 position_ws, uint sh_band, float3 direction, float radius) {

	[branch]
	if (sh_band == 0) {

		return SampleSHCoefficients(position_ws, 0, radius) * sqrt(1.f / (4.f * PI));

	}
	else if (sh_band == 1) {

		return (SampleSHCoefficients(position_ws, 1, radius) * direction.x +
				SampleSHCoefficients(position_ws, 2, radius) * direction.y +
				SampleSHCoefficients(position_ws, 3, radius) * direction.z) * sqrt(3.f / (4.f * PI));

	}
	else {

		return 0;

	}

}

float4 SampleVoxelColor(float3 position_ws, float3 direction, float radius) {

	int mip_level = GetMIPLevel(position_ws);

	if (mip_level > 0) {

		// The point is outside the domain (which causes the GetMIPLevel method to yield something greater than 0)
		return 0;

	}

	float4 color = 0.f;

	for (uint sh_band = 0; sh_band < GetSHBandCount(mip_level); ++sh_band) {

		color += SampleSHContribution(position_ws, sh_band, direction, radius);

	}

	return max(0, color.rgba);

}

float4 SampleCone(float3 origin, float3 direction, float angle, int steps) {

	float tan_angle = tan(angle * 0.5f);
	float radius;
	
	float ray_offset = gVoxelSize / (1 << gCascades);
	float4 color = 0;
	float3 position = origin;

	// Ray marching

	[flatten]
	for (int step = 0; step < steps; ++step) {

		radius = max(ray_offset * tan_angle,
					 GetMinVoxelSize(position) * 0.5f);		// No point in marching with a higher resolution than the available one

		position = origin + (ray_offset + radius) * direction;

		color += SampleVoxelColor(position, 
								  -direction, 
								  radius) /* * attenuation*/;

		ray_offset += 2 * radius;

	}

	return color;

}

#endif