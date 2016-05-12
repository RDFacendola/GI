/// \file phong_def.hlsl
/// \brief 
///
/// Eurographics Symposium on Rendering (2007) - "Microfacet Models for Refraction through Rough Surfaces" - Walter et al.
///
/// I: Direction from which light is incident. Light direction.
/// O: Direction from which light is scattered. Usually view direction.
/// N: Macrosurface normal. Surface normal.
/// M: Microsurface normal. Usually halfway direction.
///
///
/// \author Raffaele D. Facendola

#ifndef PBR_DEF_HLSL_
#define PBR_DEF_HLSL_

#include "render_def.hlsl"
#include "light_def.hlsl"

float GGXChi(float value) {

	return value > 0.f ? 1.f : 0.f;

}

float GGXDistribution(float3 M, SurfaceData surface) {

	// Trowbridge-Reitz (GGX)

	float MdotN = dot(M, surface.normal);
	
	[flatten]
	if (MdotN > 0) {

		float roughness2 = surface.roughness * surface.roughness;
	
		float MdotN2 = MdotN * MdotN;

		float denominator = MdotN2 * roughness2 + (1.f - MdotN2);

		return roughness2 / (PI * denominator * denominator);

	}
	else {

		return 0.f;

	}
		
}

float GGXGeometryPartial(float3 V, float3 M, SurfaceData surface) {

	float VdotM = dot(V, M);

	float VdotN = dot(V, surface.normal);
	
	[flatten]
	if (VdotM / VdotN > 0.f) {
		
		float VdotN2 = VdotN * VdotN;

		float tan2 = (1.f - VdotN2) / VdotN2;

		return 2.f / (1.f + sqrt(1.f + surface.roughness * surface.roughness * tan2));
		
	}
	else {

		return 0;

	}
}

float GGXGeometry(float3 I, float3 O, float3 M, SurfaceData surface) {

	return GGXGeometryPartial(I, M, surface) * GGXGeometryPartial(O, M, surface);

}

float3 GGXFresnel(float3 I, float3 M, SurfaceData surface) {

	// Dielectric materials have no absorption. This term will interpolate between the reflectance of dielectric and conductors according to the metalness of the material.
	float K = lerp(0, surface.ior.y, surface.metalness);

	float IORm = surface.ior.x - 1.0f;				// IOR - 1
	float IORp = surface.ior.x + 1.0f;				// IOR + 1
	float K2 = K * K;								// K^2

	float IORmpK2 = IORm * IORp + K2;
	float IORppK2 = IORp * IORp + K2;

	// Reflectance at normal incidence, aka surface's characteristic specular color (Cspec).

	float3 F0 = (IORmpK2 * IORmpK2 - 4 * K2) / (IORppK2 * IORppK2);		// For K = 0 it reduces to (IOR - 1)^2 / (IOR + 1)^2, which is the standard formula for dielectric

	// Dielectric have monochromatic fresnel, while material do have a specular color (we recycle their albedo here).

	F0 *= lerp(1, surface.albedo.rgb, surface.metalness);

	// Schlick's approximation
	
	return F0 + (1.f - F0) * pow(1.f - saturate(dot(I, M)), 5.f);
			
}

float3 ComputeCookTorrance(float3 light_direction, float3 view_direction, SurfaceData surface) {

	float NdotV = saturate(dot(surface.normal, view_direction));
	float NdotL = saturate(dot(surface.normal, light_direction));

	float3 halfway_direction = normalize(light_direction + view_direction);

	float3 numerator =   GGXDistribution(halfway_direction, surface)										// D
					   * GGXGeometry(light_direction, view_direction, halfway_direction, surface)			// G
					   * GGXFresnel(light_direction, halfway_direction, surface);							// F

	float denominator = max(0.001f, 4.f * NdotV /** NdotL*/);

	return (numerator /** NdotL*/) / denominator;	// * dL
	
}

float3 ComputeLambert(float3 light_direction, float3 view_direction, SurfaceData surface) {

	float NdotL = saturate(dot(surface.normal, light_direction));

	return (surface.albedo.rgb * NdotL) / PI;	// * dL

}

float3 ComputeEmissivity(SurfaceData surface) {

	return surface.albedo.rgb * surface.emissivity;

}

float3 ComputePBR(float3 light_direction, float3 view_direction, SurfaceData surface, float3 light_color) {

	float3 diffuse = ComputeLambert(light_direction, view_direction, surface);			// Lambertian diffuse

	float3 specular = ComputeCookTorrance(light_direction, view_direction, surface);	// Cook-Torrance specular (PBR).

	return (surface.kd * diffuse + surface.ks * specular * surface.cavity) * light_color;
	
}

/// \brief Compute the light contribution using a physically-based shading model
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float3 ComputePBR(SurfaceData surface, PointLight light, float3 camera_position, float shadow) {
	
	float3 L = normalize(light.position.xyz - surface.position);						// from surface to light
	float3 V = normalize(camera_position - surface.position);							// from surface to camera

	float3 surface_color = ComputePBR(L,
									  V,
									  surface,
									  light.color.rgb);

	float attenuation = saturate(GetAttenuation(light, surface.position) * shadow);								// Used to attenuate the light contribution

	return surface_color * attenuation * saturate(1.0f - surface.emissivity);

}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float3 ComputePBR(SurfaceData surface, DirectionalLight light, float3 camera_position, float shadow) {

	float3 L = -light.direction.xyz;													// from surface to light
	float3 V = normalize(camera_position - surface.position);							// from surface to camera

	float3 surface_color = ComputePBR(L,
									  V,
									  surface,
									  light.color.rgb);

	float attenuation = shadow;

	return surface_color * attenuation * saturate(1.0f - surface.emissivity);

}

#endif