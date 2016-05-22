/// \brief Setup of indirect args

RWBuffer<uint> gVoxelIndirectArguments;
RWBuffer<uint> gSHIndirectArguments;

[numthreads(1, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	gVoxelIndirectArguments[0] = 36;			// IndexCountPerInstance
	gVoxelIndirectArguments[1] = 0;				// InstanceCount***
	gVoxelIndirectArguments[2] = 0;				// StartIndexLocation
	gVoxelIndirectArguments[3] = 0;				// BaseVertexLocation
	gVoxelIndirectArguments[4] = 0;				// StartInstanceLocation

	gSHIndirectArguments[0] = 960;				// VertexCountPerInstance
	gSHIndirectArguments[1] = 0;				// InstanceCount***
	gSHIndirectArguments[2] = 0;				// StartVertexLocation
	gSHIndirectArguments[3] = 0;				// StartInstanceLocation

}