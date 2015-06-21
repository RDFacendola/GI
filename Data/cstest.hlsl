
Texture2D gAlbedo;
Texture2D gNormalShininess;

RWTexture2D<float4> uav;

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	float4 albedo = gAlbedo[thread_id.xy];
	
	float3 surface_normal = normalize(gNormalShininess[thread_id.xy].xyz);

	float i0 = saturate(dot(float3(0, 1, 0), surface_normal)) * 10.0 +				// Sky
			   saturate(dot(float3(0, -1, 0), surface_normal)) * 0.5f +				// Terrain diffuse
			   saturate(dot(float3(1, 0, 0), surface_normal)) * 1.5f +
			   saturate(dot(float3(-1, 0, 0), surface_normal)) * 1.5f +
			   saturate(dot(float3(0, 0, 1), surface_normal)) * 1.5f +
			   saturate(dot(float3(0, 0, -1), surface_normal)) * 1.5f;
	
	uav[thread_id.xy] = albedo * i0;

}