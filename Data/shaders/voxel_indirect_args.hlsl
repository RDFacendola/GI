/// \brief Setup of indirect args

RWBuffer<uint> gIndirectArguments;

[numthreads(1, 1, 1)]
void CSMain(uint3 dispatch_thread_id : SV_DispatchThreadID) {
	
	gIndirectArguments[0] = 36;				// IndexCountPerInstance
	gIndirectArguments[1] = 0;				// InstanceCount***
	gIndirectArguments[2] = 0;				// StartIndexLocation
	gIndirectArguments[3] = 0;				// BaseVertexLocation
	gIndirectArguments[4] = 0;				// StartInstanceLocation

}