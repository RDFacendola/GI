/// \file phong_def.hlsl
/// \brief This file contains the definition of Phong reflection model-related structures and functions
/// \author Raffaele D. Facendola

#ifndef PHONG_DEF_HLSL_
#define PHONG_DEF_HLSL_

#include "render_def.hlsl"
#include "light_def.hlsl"

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float4 ComputePhong(SurfaceData surface, PointLight light, float3 camera_position) {
	
	float3 light_direction = normalize(light.position.xyz - surface.position);					// from surface to light
	float3 view_direction = normalize(camera_position - surface.position);						// from surface to camera
	float3 reflection_direction = reflect(-light_direction, surface.normal);					// from surface away
	
	float4 diffuse = surface.albedo * saturate(dot(light_direction, surface.normal));			// Diffuse contribution

	float4 specular = pow(saturate(dot(reflection_direction, view_direction)),					// Specular contribution
						  surface.shininess);

	float attenuation = GetAttenuation(light, surface.position);								// [0;1]

	float4 color = (diffuse + specular) * light.color * attenuation;

	return float4(color.rgb, surface.albedo.a);		// Restore surface's alpha
	
}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float4 ComputePhong(SurfaceData surface, DirectionalLight light, float3 camera_position) {

	float3 light_direction = -light.direction.xyz;												// from surface to light
	float3 view_direction = normalize(camera_position - surface.position);						// from surface to camera
	float3 reflection_direction = reflect(-light_direction, surface.normal);					// from surface away

	float4 diffuse = surface.albedo * saturate(dot(light_direction, surface.normal));			// Diffuse contribution

	
	float4 specular = pow(saturate(dot(reflection_direction, view_direction)),					// Specular contribution
						  surface.shininess);

	float4 color = (diffuse + specular) * light.color;

	return float4(color.rgb, surface.albedo.a);		// Restore surface's alpha

}

#endif