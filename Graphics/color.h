#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
	double r = pixel_color.x();
	double g = pixel_color.y();
	double b = pixel_color.z();

	int rbyte = (255.999 * r);
	int gbyte = (255.999 * g);
	int bbyte = (255.999 * b);

	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif