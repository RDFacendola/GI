/// \file shadow_def.hlsl
/// \brief This file contains methods used to handle shadows
/// \author Raffaele D. Facendola

#ifndef SHADOW_DEF_HLSL_
#define SHADOW_DEF_HLSL_

#include "projection_def.hlsl"
#include "render_def.hlsl"

/// \brief Structure used to calculate the shadow casted by a point light.
/// The shadowmap technique used is: Dual-paraboloid variance shadow mapping.
struct PointShadow {

	float4x4 light_view_matrix;				// Matrix used to transform from world-space to light-view-space.
	
	float2 min_uv;							// Minimum uv coordinates inside the shadowmap page.
	
	float2 max_uv;							// Maximum uv coordinates inside the shadowmap page.
	
	float near_plane;						// Near clipping plane of the light.

	float far_plane;						// Far clipping plane of the light.

	int atlas_page;							// Index of the page inside the atlas containing the shadowmap.
	
	int enabled;							// Whether the shadow is enabled (!0) or not (0).
	
};

/// \brief Structure used to calculate the shadow casted by a directional light.
/// The shadowmap techinque used is: variance shadow mapping.
struct DirectionalShadow {

	float4x4 light_view_matrix;				// Matrix used to transform from world-space to light-view-space.

	float2 min_uv;							// Minimum uv coordinates inside the shadowmap page.

	float2 max_uv;							// Maximum uv coordinates inside the shadowmap page.

	int atlas_page;							// Index of the page inside the atlas containing the shadowmap.

	int enabled;							// Whether the shadow is enabled (!0) or not (0).

	float2 reserved;

};
	
Texture2DArray gVSMShadowAtlas;									// Contains the variance shadow mapping atlas. Expected format R32G32 or R16G16.

SamplerState gVSMSampler;										// Sampler used to sample the variance shadow mapping.

StructuredBuffer<PointShadow> gPointShadows;					// Buffer containing the point lights shadow.

StructuredBuffer<DirectionalShadow> gDirectionalShadows;		// Buffer containing the directional lights shadow.

/// \brief Samples from the variance shadow map atlas.
/// \param atlas_page Page containing the proper shadowmap.
/// \param min_uv Minimum uv coordinates in the shadowmap page.
/// \param max_uv Maximum uv coordinates in the shadowmap page.
/// \param uv Uv coordinates to sample.
/// \return Returns the sample of the VSM at the given uv coordinates. The X has the 1st moment, while the Y the 2nd one.
float2 SampleVSMShadowAtlas(int atlas_page, float2 min_uv, float2 max_uv, float2 uv) {

	// Remarks: the sampler is forced to have wrapping mode "clamp".

	float2 shadowmap_uv = saturate(uv) * (max_uv - min_uv) + min_uv;		// UVs in shadowmap space.

	float3 uvw = float3(shadowmap_uv, float(atlas_page));

	return gVSMShadowAtlas.SampleLevel(gVSMSampler, uvw, 0).xy;
		
}

/// \brief Computes the variance shadow map lit factor.
/// \param moments Contains the moments stored inside the VSM.
/// \param depth Depth of the surface to test.
/// \return Returns a value in the range 1 (fully lit) to 0 (fully shadowed).
float ComputeVSMFactor(float2 moments, float depth) {

	// http://developer.download.nvidia.com/SDK/10/direct3d/Source/VarianceShadowMapping/Doc/VarianceShadowMapping.pdf

	if (depth < moments.x) {

		return 1.f;

	}

	float variance = moments.y - moments.x * moments.x;		// E[x^2] - E[x]^2

	float mD = moments.x - depth;							// Difference from the expected depth value E[x]

	return saturate(variance / (variance + mD * mD));		// Chebyshev's inequality. It's an upper bound: it may lead to false positives.

}

/// \brief Computes the visibility of the surface from the given point light.
/// \param surface Coordinates of the surface in world space.
/// \return Returns a value in the range 1 (fully lit) to 0 (fully shadowed).
float ComputeShadow(SurfaceData surface, PointShadow shadow) {

	if (shadow.enabled == 0) {

		return 1.0f;		// Fully lit

	}

	float4 surface_ls = mul(shadow.light_view_matrix, float4(surface.position, 1));				// Surface point in light-space.

	float2 min_uv = shadow.min_uv;
	float2 max_uv = shadow.max_uv;
	
	if (surface_ls.z >= 0.f) {

		// Front paraboloid
		surface_ls.z *= 1.0f;
		max_uv.x = (max_uv.x + min_uv.x) * 0.5f;

	}
	else {

		// Rear paraboloid
		surface_ls.z *= -1.0f;
		min_uv.x = (max_uv.x + min_uv.x) * 0.5f;

	}

	float4 surface_ps = ProjectToParaboloidSpace(surface_ls.xyz, 
												 shadow.near_plane, 
												 shadow.far_plane);								// Surface in paraboloid space. XY contains the uv coordinates, Z the depth of the point.
		
	float2 moments = SampleVSMShadowAtlas(shadow.atlas_page, 
										  min_uv, 
										  max_uv, 
										  ProjectionSpaceToTextureSpace(surface_ps));			// Get the shadow moments.

	return ComputeVSMFactor(moments, surface_ps.z);												// Get the lit factor via Chebyshev's inequality.

}

/// \brief Computes the visibility of the surface from the given directional light.
/// \param surface Coordinates of the surface in world space.
/// \return Returns a value in the range 1 (fully lit) to 0 (fully shadowed).
float ComputeShadow(SurfaceData surface, DirectionalShadow shadow) {

	return 1.0f;				// Not yet implemented.

}

#endif