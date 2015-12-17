#include "render_def.hlsl"
#include "light_def.hlsl"
#include "phong_def.hlsl"

#include "shadow_def.hlsl"

cbuffer gParameters {

	float4x4 inv_view_proj_matrix;		///< \brief Inverse view-projection matrix.
	float4 camera_position;				///< \brief Camera position in world space.
	uint point_lights;					///< \brief Number of point lights.
	uint directional_lights;			///< \brief Number of directional lights.

};

// Light accumulation buffer
RWTexture2D<float4> gLightAccumulation;

[numthreads(16,16,1)]
void CSMain(int3 thread_id : SV_DispatchThreadID){

	uint light_index;

	SurfaceData surface = GatherSurfaceData(thread_id.xy, inv_view_proj_matrix);

	float3 color = ComputeEmissivity(surface);

	// Accumulate point lights
	for (light_index = 0; light_index < point_lights; ++light_index) {

		color += ComputePhong(surface, 
							  gPointLights[light_index], 
							  camera_position.xyz, 
							  ComputeShadow(surface, gPointShadows[light_index]));

	}

	// Accumulate directional lights
	for (light_index = 0; light_index < directional_lights; ++light_index) {

		color += ComputePhong(surface, 
							  gDirectionalLights[light_index], 
							  camera_position.xyz, 
							  ComputeShadow(surface, gDirectionalShadows[light_index]));

	}

	// Done

	gLightAccumulation[thread_id.xy] = float4(color, 1);	// Opaque
	
}
