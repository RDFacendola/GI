/// \file hlsl_def.hlsl
/// \brief This file contains general HLSL methods.
/// \author Raffaele D. Facendola

#ifndef HLSL_DEF_HLSL_
#define HLSL_DEF_HLSL_

/// Get the sign of the specified value as bit mask
/// 100: negative
/// 010: zero
/// 001: positive
/// The method sets only one bit
int GetSignMask(float value) {

	return (1 << (1 - sign(value))) & 0x7;

}

/// Get the composite sign mask of the specified values
/// 1XX: at least one value was negative
/// X1X: at least one value was zero
/// XX1: at least one value was positive
int GetSignMask(float a, float b, float c) {

	return GetSignMask(a) | GetSignMask(b) | GetSignMask(c);

}

#endif