/// \brief Contains functions for color manipulation.
/// \author Raffaele D. Facendola

/// \brief Gets the relative luminance of a color.
float RelativeLuminance(float3 color) {

	// https://en.wikipedia.org/wiki/Relative_luminance

	return dot(color.rbg,
			   float3(0.2126f, 0.7152f, 0.0722f));

}