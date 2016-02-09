#include "common/hlsl_def.hlsl"
#include "projection_def.hlsl"

cbuffer Parameters {

	float3 gCenter;								// Center of the voxelization. It is always a corner shared among 8 different voxels.

	float gVoxelSize;							// Size of each voxel in world units for each dimension.

	unsigned int gVoxelResolution;				// Resolution of each cascade in voxels for each dimension.

	unsigned int gCascades;						// Number of additional cascades inside the clipmap.
			
};

/////////////////////////////////// VERTEX SHADER ///////////////////////////////////////

cbuffer PerObject {

	float4x4 gWorld;							// World matrix of the object to voxelize, in grid space.

};

float4 VSMain(float4 position : SV_Position) : SV_Position{
	
	float4 pos = mul(gWorld, position);									// World space
	
	float grid_size = gVoxelSize * gVoxelResolution;

	return (pos - float4(gCenter, 0.0f)) * (2.f / grid_size);			// Voxel space [-1;+1]

}

/////////////////////////////////// GEOMETRY SHADER ////////////////////////////////////

struct GSOut {

	float4 position_ps: SV_Position;					// Vertex position in voxel-grid projection space.

	unsigned int projection_plane : ProjectionPlane;	// Projection plane. 0: (Right) ZY(-X), 1: (Above) XZ(-Y), 2: (Front) XY(Z)

	unsigned int cascade : Cascade;						// Voxel cascade index.

};

// 5 Cascades maximum (3*5 vertices)
[maxvertexcount(15)]
void GSMain(triangle float4 input[3] : SV_Position, inout TriangleStream<GSOut> output_stream) {
	
	// TODO: Optimize the branchiness here

	GSOut output;

	// Normal of the triangle N = (A-B) x (C-B), P = (A-B), Q = (C-B)
	//
	// N = <PyQz - PzQy,
	//	    PxQz - PzQx,
	//      PxQy - PyQx>
	//
	//
	// Which d in {<1;0;0>, <0;1;0>, <0;0;1>} maximizes |d dot N| ? i.e. i | N[i] >= N[j] forall J in [0;2]

	float3 abs_normal = abs(cross(input[1].xyz - input[0].xyz,
								  input[2].xyz - input[0].xyz));

	// Determine which axis maximizes the rasterized area and shuffle the polygon coordinates accordingly

	if (abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z) {

		// From the right: -X is the depth
		output.projection_plane = 0;

		input[0] = input[0].zyxw;
		input[1] = input[1].zyxw;
		input[2] = input[2].zyxw;

	}
	else if (abs_normal.y > abs_normal.z) {

		// From the above: -Y is the depth
		output.projection_plane = 1;

		input[0] = input[0].xzyw;
		input[1] = input[1].xzyw;
		input[2] = input[2].xzyw;

	}
	else {
		
		// From the front: +Z is the depth
		output.projection_plane = 2;

		//input[0] = input[0].xyzw;
		//input[1] = input[1].xyzw;
		//input[2] = input[2].xyzw;

	}

	// Output one primitive per cascade.
	// In each cascade the primitive is shrunk by a factor of 2 with respect to the last iteration

	for (unsigned int cascade_index = 0; cascade_index <= gCascades; ++cascade_index) {

		[unroll]
		for (unsigned int vertex_index = 0; vertex_index < 3; ++vertex_index) {

			output.position_ps = float4(input[vertex_index].x,						// [-1;+1]
										input[vertex_index].y * -1.0f,				// [+1;-1]. Compensate for the V axis which grows downwards.
										input[vertex_index].z * 0.5f + 0.5f,		// [ 0;+1]. Will kill geometry outside the Z boundaries
										1.0f);

			output.position_ps.xyz /= (1 << cascade_index);

			output.cascade = cascade_index;

			output_stream.Append(output);

		}

		output_stream.RestartStrip();
		
	}

}

/////////////////////////////////// PIXEL SHADER ///////////////////////////////////////

RWStructuredBuffer<uint> gVoxelAddressTable;			///< \brief Clipmap containing the address of the data stored for each voxel.

float4 PSMain(GSOut input) : SV_Target0{

	uint linear_coordinate;		// Linear coordinate of the voxel within its cascade.

	// Restore the unshuffled coordinates from the GS

	input.position_ps.z *= (gVoxelResolution - 1);

	if (input.projection_plane == 0) {
		
		input.position_ps = input.position_ps.zyxw;

	}
	else if (input.projection_plane == 1) {

		input.position_ps = input.position_ps.xzyw;

	}
	else {
				
		input.position_ps = input.position_ps.xyzw;
		
	}
	
	linear_coordinate = floor(input.position_ps.x) +
						floor(input.position_ps.y) * gVoxelResolution +
						floor(input.position_ps.z) * gVoxelResolution * gVoxelResolution;
						
	// The cascades are stored at the end of the structure, so we can just start from the back

	uint VAT_elements;
	uint dummy;

	gVoxelAddressTable.GetDimensions(VAT_elements, dummy);

	uint cascade_size = gVoxelResolution * gVoxelResolution * gVoxelResolution;		// Size of each cascade, in voxels

	uint pyramid_size = VAT_elements - cascade_size * (gCascades + 1);				// Size of the pyramid, without its last level. Elements inside the pyramid are always ignored.
		
	linear_coordinate += pyramid_size + (cascade_size * input.cascade);

	// Reserve the next free address from the address table

	InterlockedAdd(gVoxelAddressTable[linear_coordinate], 1, dummy);		// TODO: Put the actual address here
	
	return 1.0f;		// Not needed!

}
