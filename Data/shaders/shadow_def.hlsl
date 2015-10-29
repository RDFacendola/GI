/// \file shadow_def.hlsl
/// \brief This file contains methods used to handle shadows
/// \author Raffaele D. Facendola

#ifndef SHADOW_DEF_HLSL_
#define SHADOW_DEF_HLSL_

#include "projection_def.hlsl"
#include "render_def.hlsl"



float VarianceShadowMap(Texture2D shadow_map, float2 position_ts, float depth) {

	// http://developer.download.nvidia.com/SDK/10/direct3d/Source/VarianceShadowMapping/Doc/VarianceShadowMapping.pdf

	float2 moments = 0.0f;

	//moments = tex2D(shadow_map, position_ts.xy).xy;

	if (depth < moments.x) {

		return 1.0f;		// Closer than the average depth => fully lit

	}

	float variance = moments.y - moments.x * moments.x;		// E[x^2] - E[x]^2

	float mD = moments.x - depth;							// Difference from the expected depth value E[x]
	
	return variance / (variance + mD * mD);					// Chebyshev's inequality. It's an upper bound: it may lead to false positives.

}

#endif