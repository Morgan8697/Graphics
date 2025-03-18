#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"

/*
 * Color Utilities for Ray Tracing
 * -------------------------------
 * Provides functions to handle color conversion and output for ray-traced images.
 * Colors are represented as vec3, with components in the [0, 1] range.
 *
 * Purpose:
 * --------
 * - Convert linear color values to gamma-corrected values for proper brightness display.
 * - Clamp color values to valid displayable range.
 * - Output pixel color in PPM format for image rendering.
 *
 * Gamma Correction:
 * -----------------
 * - Human eyes perceive light non-linearly; gamma correction accounts for this.
 * - Converts linear color values to gamma 2.0 space using sqrt(linear_value).
 * - Ensures images look natural on standard displays.
 *
 * Functionality:
 * --------------
 * - `linear_to_gamma(double)`: Applies gamma correction to one color component.
 * - `write_color(ostream, color)`: Converts a color to 8-bit format and writes it to an output stream.
 *
 * Usage:
 * ------
 * color pixel_color(0.5, 0.7, 1.0);
 * write_color(std::cout, pixel_color);
 *
 * Notes:
 * ------
 * - Clamps color values to [0, 0.999] before converting to 8-bit.
 * - Assumes final output is in the plain PPM (P3) image format.
 */


using color = vec3;

inline double linear_to_gamma(double linear_component)
{
	if (linear_component > 0)
		return std::sqrt(linear_component);

	return 0;
}

void write_color(std::ostream& out, const color& pixel_color) {
	double r = pixel_color.x();
	double g = pixel_color.y();
	double b = pixel_color.z();

	// Apply a linear to gamma transform for gamma 2
	r = linear_to_gamma(r);
	g = linear_to_gamma(g);
	b = linear_to_gamma(b);

	// Translate the [0,1] component values to the byte range [0,255].
	static const interval intensity(0.000, 0.999);
	int rbyte = (255.999 * intensity.clamp(r));
	int gbyte = (255.999 * intensity.clamp(g));
	int bbyte = (255.999 * intensity.clamp(b));

	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif