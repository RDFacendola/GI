/// \file encode_def.hlsl
/// \brief This file contains encoding\decoding methods
/// \author Raffaele D. Facendola

#ifndef ENCODE_DEF_HLSL_
#define ENCODE_DEF_HLSL_

/// \see http://jcgt.org/published/0003/02/01/paper.pdf
float2 OctWrap(float2 v){

	return (1.0 - abs(v.yx)) * (v.xy >= 0.f ? 1.f : -1.f);

}

/// \brief Encodes a 3-element vector to a 2-element octahedron-encoded vector.
float2 EncodeNormals(float3 decoded) {

	decoded /= (abs(decoded.x) + abs(decoded.y) + abs(decoded.z));

	decoded.xy = (decoded.z >= 0.f) ? decoded.xy : OctWrap(decoded.xy);

	decoded.xy = decoded.xy * 0.5f + 0.5f;

	return decoded.xy;

}

/// \brief Decodes a 2-element octahedron-encoded vector to a 3-element vector.
float3 DecodeNormals(float2 encoded) {

	encoded = encoded * 2.f - 1.f;

	float3 decoded;

	decoded.z = 1.0 - abs(encoded.x) - abs(encoded.y);

	decoded.xy = (decoded.z >= 0.f) ? encoded.xy : OctWrap(encoded.xy);
	
	return normalize(decoded);
	
}

/// \brief Encodes a 3-element vector to a float-encoded vector.
float EncodeNormalsCoarse(float3 decoded) {

	static const uint kStride = 16;

	uint2 encoded = EncodeNormals(decoded) * (kStride - 1);

	uint linear_encoded = encoded.x + encoded.y * kStride;

	return (float)(linear_encoded) / (kStride * kStride);

}

/// \brief Decodes a float-encoded vector to a 3-element vector.
float3 DecodeNormalsCoarse(float encoded) {

	static const uint kStride = 16;

	uint linear_encoded = encoded * (kStride * kStride);
	
	uint2 decoded = uint2(linear_encoded % kStride,
						  linear_encoded / kStride);

	return DecodeNormals((float2)(decoded) / (kStride - 1));

}

#endif