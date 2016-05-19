/// \file voxel_def.hlsl
/// \brief This file contains shared definition for voxels
///
/// \author Raffaele D. Facendola

#ifndef VOXEL_VOXEL_DEF_HLSL_
#define VOXEL_VOXEL_DEF_HLSL_

#define PI 3.14159f

SamplerState gSHSampler;				// Sampler used to sample the SH

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

	uint3 sh_address;					// Address of the SH coefficients. Without the cascade!

	int cascade;						// Cascade the voxel falls in. A negative number indicates a MIP map.

	uint sh_bands;						// Number of bands used by the voxel.

	uint3 padding;

};

/// \brief Get the number of SH bands stored for each voxel in a specified cascade.
uint GetSHBandCount(uint cascade_index) {

	return 2;		// 2 SH bands, for now

}

/// \brief Get the total amount of voxels inside a cascade
uint GetCascadeSize() {

	return gVoxelResolution * gVoxelResolution * gVoxelResolution;

}

/// \brief Get the total amount of voxels inside the pyramid of the voxel clipmap, without the last level.
uint GetClipmapPyramidSize() {

	// Simplified form of a sum of a geometric series S = (1 - 8^log2(gVoxelResolution)) / (1 - 8)

	return (GetCascadeSize() - 1) / 7;

}

/// \brief Get the voxel size given its cascade.
/// \param cascade Cascade the voxel belong to. Negative cascade are inside the clipmap pyramid.
float GetVoxelSize(int cascade) {

	return gVoxelSize * pow(2.0f, -cascade);

}

/// \brief Get the minimum size of a voxel inside the structure.
float GetMinVoxelSize() {

	return gVoxelSize / (float)(1 << gCascades);

}

/// \brief Get the cascade index of a point in world space.
int GetCascade(float3 position_vs) {
	
	float3 abs_position = abs(position_vs);

	float max_distance = max(abs_position.x, max(abs_position.y, abs_position.z));

	float voxel_distance = max_distance * rcp(GetMinVoxelSize());			// Distance in "voxel" units

	float cascade_distance = floor(voxel_distance * rcp(gVoxelResolution >> 1)) + 1;

	return ((int)gCascades) - ceil(log2(cascade_distance));

}

/// \brief Get the minimum size of a voxel around the specified point.
float GetMinVoxelSize(float3 position_ws) {

	return GetVoxelSize(GetCascade(position_ws - gCenter));

}

int3 ToIntSH(float3 sh_coefficient) {

	return sh_coefficient * 50;

}

float4 ToFloatSH(int4 sh_coefficient) {

	return sh_coefficient * 0.001f;

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

	bool is_inside_clipmap = index >= GetClipmapPyramidSize() && 
							 index < GetClipmapPyramidSize() + GetCascadeSize() * (1 + gCascades);

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
	
	voxel_info.cascade = index / GetCascadeSize();
	
	voxel_info.size = gVoxelSize / pow(2, voxel_info.cascade);
	
	voxel_info.center = (((int3)(voxel_ptr3) - (int)(gVoxelResolution >> 1)) + 0.5f) * voxel_info.size + gCenter;

	voxel_info.sh_address = voxel_ptr3 % gVoxelResolution;

	voxel_info.sh_bands = 2;		// 2 bands, for now

	voxel_info.padding = 0;

	return true;

}

/// \brief Get the info of a voxel in a given point in space.
/// \param voxel_address_table Linear array containing the voxel pointer structure.
/// \param position_ws Coordinates of the point, in world space.
/// \param voxel_info If the method succeeds it contains the requested voxel informations.
/// \return Returns true if the point was in the voxelized domain, returns false otherwise.
bool GetVoxelInfo(StructuredBuffer<uint> voxel_address_table, float3 position_ws, out VoxelInfo voxel_info) {

	position_ws -= gCenter;				// To voxel-grid space

	voxel_info.cascade = GetCascade(position_ws);
	
	voxel_info.size = GetVoxelSize(voxel_info.cascade);

	int3 voxel_coordinates = floor(position_ws * rcp(voxel_info.size));		//[-R/2; R/2)

	voxel_info.center = (voxel_coordinates + 0.5f) * voxel_info.size + gCenter;

	voxel_info.sh_address = voxel_coordinates + (gVoxelResolution >> 1);

	voxel_info.sh_bands = GetSHBandCount(voxel_info.cascade);

	voxel_info.padding = 0;

	return voxel_info.cascade >= 0;		// A negative cascade means that the provided world position is beyond the voxelized scene

}

/// \brief Get the 3D coordinates of the most precise version of the voxel enclosing the specified point.
/// \param position_ws Position of the point, in world space.
/// \param coefficient_index Index of the SH coefficient.
/// \param address If the method succeeds, it contains the address for each of the channels R, G, B.
/// \return Returns 0 if the voxel couldn't be found, 1 if it was found inside the pyramid part of the clipmap or 2 if it was found inside the stack part of the clipmap.
int GetSHCoordinates(float3 position_ws, uint coefficient_index, out uint3 address[3]) {

	float3 position_vs = position_ws - gCenter;

	int cascade = GetCascade(position_vs);

	float voxel_size = GetVoxelSize(cascade);

	address[0] = floor(position_vs * rcp(voxel_size)) + (gVoxelResolution * 0.5f);			// Red
	
	address[0].x += coefficient_index * gVoxelResolution;									// Move to the correct coefficient
	address[0].y += sign(cascade) * (cascade - 1) * gVoxelResolution;						// Move to the correct MIP. The sign(.) here is to bypass the instruction if we are already inside the pyramid.
	
	address[1] = address[0] + uint3(0, 0, 1 * gVoxelResolution);							// Green
	address[2] = address[0] + uint3(0, 0, 2 * gVoxelResolution);							// Blue

	return sign(cascade) + 1;		// 0 for cascade < 0
									// 1 for cascade = 0
									// 2 for cascade > 0

}

/// \brief Store the first two band of SH coefficients
void StoreSHCoefficients(RWTexture3D<int> sh_pyramid, RWTexture3D<int> sh_stack, float3 position_ws, uint coefficient_index, float3 sh_coefficients) {

	uint3 address[3];

	int3 coefficients = ToIntSH(sh_coefficients);

	int result = GetSHCoordinates(position_ws,
								  coefficient_index, 
								  address);

	if (result == 1) {

		// Pyramid part

		InterlockedAdd(sh_pyramid[address[0]], coefficients.r);
		InterlockedAdd(sh_pyramid[address[1]], coefficients.g);
		InterlockedAdd(sh_pyramid[address[2]], coefficients.b);

	}
	else if (result == 2) {

		// Stack part

		InterlockedAdd(sh_stack[address[0]], coefficients.r);
		InterlockedAdd(sh_stack[address[1]], coefficients.g);
		InterlockedAdd(sh_stack[address[2]], coefficients.b);

	}
	else {

		// Outside domain, do nothing

	}

}

int GetSHCoordinates(float3 position_vs, uint coefficient_index, int cascade, out float3 address) {

	float voxel_size = GetVoxelSize(cascade);

	address = position_vs * rcp(voxel_size) + (gVoxelResolution * 0.5f);

	address.x += coefficient_index * gVoxelResolution;					// Move to the correct coefficient
	address.y += sign(cascade) * (cascade - 1) * gVoxelResolution;		// Move to the correct MIP. The sign(.) here is to bypass the instruction if we are already inside the pyramid.

	return sign(cascade) + 1;		// 0 for cascade < 0
									// 1 for cascade = 0
									// 2 for cascade > 0

}

float4 SampleSHCoefficients(Texture3D<float4> sh_pyramid, Texture3D<float4> sh_stack, float3 position_ws, uint sh_index, float radius) {

	float3 dimensions;
	int coefficients;

	float3 position_vs = position_ws - gCenter;
	
	float voxel_size = radius * 2.0f;

	// Assumption: radius is always at least the maximum resolution available for the specified sample location. Avoids multiple GetMinVoxelSize(.)

	if (voxel_size >= gVoxelSize) {

		// Sample directly from the pyramid. 
		// Optimal case: we rely on the hardware quadrilinear filtering.

		sh_pyramid.GetDimensions(dimensions.x,				// gVoxelResolution * #Coefficients
								 dimensions.y,				// gVoxelResolution
								 dimensions.z);				// gVoxelResolution

		coefficients = dimensions.x / gVoxelResolution;		// Number of SH coefficients

		float3 sample_location = (position_vs / (gVoxelSize * gVoxelResolution)) + 0.5f;	// [0; 1]

		sample_location.x = (sample_location.x + sh_index) / coefficients;					// Move to the correct SH coefficient.
		
		float mip_level = log2(voxel_size / gVoxelSize);
		
		return sh_pyramid.SampleLevel(gSHSampler, sample_location, mip_level);				// Sample & interpolate

	}
	else {

		// Sample twice from the stack or one time from the stack and the other from the pyramid.
		// Bad case: we sample twice and perform a linear interpolation in software.

		return float4(0, 0, 0, 1);

	}

	//float3 address;
	//
	//int cascade = GetCascade(position_vs);

	//int result = GetSHCoordinates(position_vs, 
	//							  sh_index, 
	//							  cascade,
	//							  address);

	//if (result == 1) {

	//	// Pyramid
	//	sh_pyramid.GetDimensions(dimensions.x,			// gVoxelResolution * #Coefficients
	//							 dimensions.y,			// gVoxelResolution
	//							 dimensions.z);			// gVoxelResolution

	//	float3 sample_location = address / dimensions;

	//	return sh_pyramid.SampleLevel(gSHSampler, sample_location, 0);
	//	
	//}
	//else if (result == 2) {
	//	
	//	// Stack

	//	sh_stack.GetDimensions(dimensions.x,			// gVoxelResolution * #Coefficients
	//						   dimensions.y,			// gVoxelResolution * #Cascades
	//						   dimensions.z);			// gVoxelResolution

	//	float3 sample_location = address / dimensions;

	//	return sh_stack.SampleLevel(gSHSampler, sample_location, 0);

	//}
	//else {
	//
	//	// Outside the domain

	//	return 0;		

	//}
			
}

float4 SampleSHContribution(Texture3D<float4> sh_pyramid, Texture3D<float4> sh_stack, float3 position_ws, uint sh_band, float3 direction, float radius) {

	[branch]
	if (sh_band == 0) {

		return SampleSHCoefficients(sh_pyramid, sh_stack, position_ws, 0, radius) * sqrt(1.f / (4.f * PI));

	}
	else if (sh_band == 1) {

		return (SampleSHCoefficients(sh_pyramid, sh_stack, position_ws, 1, radius) * direction.x +
				SampleSHCoefficients(sh_pyramid, sh_stack, position_ws, 2, radius) * direction.y +
				SampleSHCoefficients(sh_pyramid, sh_stack, position_ws, 3, radius) * direction.z) * sqrt(3.f / (4.f * PI));

	}
	else {

		return 0;

	}

}

float4 SampleVoxelColor(Texture3D<float4> sh_pyramid, Texture3D<float4> sh_stack, float3 position_ws, float3 direction, float radius) {

	int cascade = GetCascade(position_ws);

	float4 color = 0.f;

	for (uint sh_band = 0; sh_band < GetSHBandCount(cascade); ++sh_band) {

		color += SampleSHContribution(sh_pyramid, sh_stack, position_ws, sh_band, direction, radius);

	}

	return max(0, color.rgba);

}

float4 SampleCone(Texture3D<float4> sh_pyramid, Texture3D<float4> sh_stack, float3 origin, float3 direction, float angle, int steps) {

	float tan_angle = tan(angle * 0.5f);
	float radius;
	
	float ray_offset = GetMinVoxelSize();
	float4 color = 0;
	float3 position = origin;

	// Ray marching

	[flatten]
	for (int step = 0; step < steps; ++step) {

		radius = max(ray_offset * tan_angle,
					 GetMinVoxelSize(position) * 0.5f);		// No point in marching with a higher resolution than the available one

		position = origin + (ray_offset + radius) * direction;

		color += SampleVoxelColor(sh_pyramid, 
								  sh_stack, 
								  position, 
								  -direction, 
								  radius) /* * attenuation*/;

		ray_offset += 2 * radius;

	}

	return color;

}

#endif