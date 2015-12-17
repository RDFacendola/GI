/// \file phong_def.hlsl
/// \brief This file contains the definition of Phong reflection model-related structures and functions
///
/// The phong reflection model is with minimum variations is used here.
///
/// Thing to keep in mind:
/// 
/// The light and the surface have no "specular color" but just a greyscale value (surface.specular). This is ok for most materials out there.
/// A surface can either diffuse/reflect light or emit light. The value of diffused/reflected light and emitted light is determined by the emissivity of the surface.
/// A fully emissive surface (emissivity > 1) has no diffuse/specular component
///
/// \author Raffaele D. Facendola

#ifndef PHONG_DEF_HLSL_
#define PHONG_DEF_HLSL_

#include "render_def.hlsl"
#include "light_def.hlsl"

/// \brief Compute the illumination based on the Phong reflectance model.
/// \param light_direction Direction from the surface to the light.
/// \param camera_position Camera position in world space.
float3 ComputePhong(float3 light_direction, float3 camera_position, SurfaceData surface) {

	float3 view_direction = normalize(camera_position - surface.position);											// from surface to camera
	float3 reflection_direction = reflect(-light_direction, surface.normal);										// from surface away

	float3 diffuse = surface.albedo.rgb * saturate(dot(light_direction, surface.normal));							// Diffuse contribution

	float3 specular = pow(saturate(dot(reflection_direction, view_direction)),										// Specular contribution
						  surface.shininess) * surface.specular;

	float3 color = diffuse + specular;

	return color.rgb;

}

/// \brief Compute the emissivity of the surface.
float3 ComputeEmissivity(SurfaceData surface) {

	return surface.albedo.rgb * surface.emissivity;

}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float3 ComputePhong(SurfaceData surface, PointLight light, float3 camera_position, float shadow) {

	float3 surface_color = ComputePhong(normalize(light.position.xyz - surface.position),
										camera_position,
										surface);

	float attenuation = GetAttenuation(light, surface.position) * shadow;								// Used to attenuate the light contribution

	return surface_color * light.color.rgb * attenuation * saturate(1.0f - surface.emissivity);

}

/// \brief Compute the light contribution using the Phong reflection model.
/// \param surface Surface informations.
/// \param light Light shining on the surface.
/// \param camera_position View position.
float3 ComputePhong(SurfaceData surface, DirectionalLight light, float3 camera_position, float shadow) {

	float3 surface_color = ComputePhong(-light.direction.xyz,
										camera_position,
										surface);

	float attenuation = shadow;

	return surface_color * light.color.rgb * attenuation * saturate(1.0f - surface.emissivity);

}

#endif