/// \file phong_def.hlsl
/// \brief This file contains the definition of Phong reflection model-related structures and functions
/// \author Raffaele D. Facendola

#ifndef PHONG_DEF_HLSL_
#define PHONG_DEF_HLSL_

#include "render_def.hlsl"
#include "light_def.hlsl"

/// \brief Compute the illumination based on the Phong reflectance model.
/// \param light_direction Direction from the surface to the light.
/// \param camera_position Camera position in world space.
float3 ComputePhong(float3 light_direction, float3 camera_position, SurfaceData surface) {

	float3 view_direction = normalize(camera_position - surface.position);						// from surface to camera
	float3 reflection_direction = reflect(-light_direction, surface.normal);					// from surface away

	float4 diffuse = surface.albedo * saturate(dot(light_direction, surface.normal));			// Diffuse contribution

	float4 specular = pow(saturate(dot(reflection_direction, view_direction)),					// Specular contribution
						  surface.shininess);

	float4 color = diffuse + specular * 0.2f;

	return color.rgb;

}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float4 ComputePhong(SurfaceData surface, PointLight light, float3 camera_position) {

	float3 color = ComputePhong(normalize(light.position.xyz - surface.position),
								camera_position,
								surface);

	float attenuation = GetAttenuation(light, surface.position);								// [0;1]

	return float4(color * light.color.rgb * attenuation,										// Attenuate light contribution
				  surface.albedo.a);															// Restore surface's alpha

}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float4 ComputePhong(SurfaceData surface, DirectionalLight light, float3 camera_position) {

	float3 color = ComputePhong(-light.direction.xyz,
								camera_position,
								surface);

	return float4(color * light.color.rgb, 
				  surface.albedo.a);						// Restore surface's alpha

}

#endif