/// \brief Contains functions for color manipulation.
/// \author Raffaele D. Facendola

/// \brief Gets the relative luminance of a color.

float Luminance(float3 color) {

	// http://www.scantips.com/lumin.html

	return max(dot(color, 
				   float3(0.3f, 0.59f, 0.11f)),
			   0.0001f);

}