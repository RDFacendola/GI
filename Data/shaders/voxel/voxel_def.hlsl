/// \file voxel_def.hlsl
/// \brief This file contains shared definition for voxels
///
/// \author Raffaele D. Facendola

#ifndef VOXEL_VOXEL_DEF_HLSL_
#define VOXEL_VOXEL_DEF_HLSL_

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

};

#endif