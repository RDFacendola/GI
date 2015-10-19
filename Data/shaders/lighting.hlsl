
// GBuffer surfaces
Texture2D gAlbedo;
Texture2D gNormalShininess;

// Lights

// Light accumulation buffer
RWTexture2D<float4> gLightAccumulation;

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	float4 albedo = gAlbedo[thread_id.xy];

	float3 surface_normal = normalize(gNormalShininess[thread_id.xy].xyz);

	float4 sun_albedo = float4(0.89f, 0.66f, 0.34f, 0.0f);		// float4(227, 168, 87, 0)

	float4 sky_contribution = saturate(dot(float3(-0.71f, 0.71f, 0), surface_normal)) * sun_albedo * 3.5f;
	
	float4 global_contribution = 0.1f;

	float4 final_color = albedo * (sky_contribution + global_contribution);


	gLightAccumulation[thread_id.xy] = float4(final_color.rgb, 1);				// Alpha fixed to opaque

}
