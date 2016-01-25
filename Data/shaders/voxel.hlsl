#include "common/hlsl_def.hlsl"
#include "projection_def.hlsl"

cbuffer Parameters {

	unsigned int gVoxelResolution;				// Resolution of each cascade in voxels for each dimension

	unsigned int gCascades;						// Number of additional cascades inside the clipmap

	float gVoxelSize;							// Size of each voxel in world units.

};

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

cbuffer PerObject {

	float4x4 gWorld;							// World matrix of the object to voxelize.

};

float4 VSMain(float4 position : SV_Position) : SV_Position{
	
	return mul(gWorld, position);

}

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

//struct GSOut {
//
//	float4 position_ps: SV_Position;					// Vertex position in voxel-grid projection space.
//
//	unsigned int projection_plane : ProjectionPlane;	// Projection plane
//
//};
//
//[maxvertexcount(40)]
//void GSMain(triangle float4 input[3] : SV_Position, inout TriangleStream<GSOut> output_stream) {
//
//	// The geometry shader will find the axis along X Y and Z which maximizes the rasterized area of the pixel shader 
//	// and will output at most one polygon per clipmap cascade.
//
//
//}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

RWStructuredBuffer<uint> gVoxelAddressTable;			///< \brief Clipmap containing the address of the data stored for each voxel.

float4 PSMain(float4 input : SV_Position) : SV_Target0{

	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html

	gVoxelAddressTable[0] = gVoxelResolution;
	gVoxelAddressTable[1] = gCascades;
	gVoxelAddressTable[2] = gVoxelSize;

	return float4(1, 0, 0, 0);

}
