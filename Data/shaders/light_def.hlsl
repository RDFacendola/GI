/// \file light_def.hlsl
/// \brief This file contains the definition of light structures
/// \author Raffaele D. Facendola

#ifndef LIGHT_DEF_HLSL_
#define LIGHT_DEF_HLSL_

/// \brief Describes a single point light.
struct PointLight {

	float4 position;		// Position of the light in world space.
	float4 color;			// Color of the light.
	float kc;				// Constant attenuation factor.
	float kl;				// Linear attenuation factor.
	float kq;				// Quadratic attenuation factor.
	float reserved;

};

/// \brief Describes a single directional light.
struct DirectionalLight {

	float4 direction;		// Normal of the light in world space.
	float4 color;			// Color of the light.

};

/// Light arrays

StructuredBuffer<PointLight> gPointLights;
StructuredBuffer<DirectionalLight> gDirectionalLights;

/// \brief Computes the attenuation of a point light source hitting a surface.
/// \param light Light source.
/// \param surface Coordinates of the surface, in world space.
float GetAttenuation(PointLight light, float3 surface) {

	float d = distance(light.position.xyz, surface);

	return saturate(1.0f / (light.kc + light.kl * d + light.kq * d * d));

}

#endif