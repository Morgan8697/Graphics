#ifndef PERLIN_H
#define PERLIN_H

#include "helper.h"

/*
 * Perlin Noise Generator
 * ----------------------
 * Generates smooth, repeatable 3D Perlin-style noise used for procedural textures.
 * Incorporates trilinear interpolation and Hermitian smoothing for visual quality.
 *
 * Purpose:
 * --------
 * - Produces coherent noise where nearby 3D points return similar values.
 * - Used for rendering natural textures like marble, wood, clouds.
 *
 * Key Features:
 * -------------
 * - `noise(point3 p)`: Returns a smooth noise value in [0, 1) for input 3D point `p`.
 * - Uses 3D lattice of pseudo-random values and interpolates via `trilinear_interp()`.
 * - Applies Hermitian cubic smoothing (u = u*u*(3-2u)) to reduce visual artifacts.
 * - Repeatable via fixed-size permutation tables (perm_x, perm_y, perm_z).
 *
 * Interpolation Details:
 * ----------------------
 * - Computes noise contribution from the 8 corners of a surrounding cube.
 * - Interpolates using weights derived from fractional part of `p`.
 * - Hermitian smoothing refines interpolation to reduce grid artifacts (e.g., Mach bands).
 *
 * Usage:
 * ------
 * perlin noise_gen;
 * double val = noise_gen.noise(point3(x, y, z));
 *
 * Notes:
 * ------
 * - Efficient for real-time procedural texture generation.
 * - Higher-quality variations can extend to gradient noise or turbulence.
 */



class perlin {
public:
	perlin() {
		for (int i = 0; i < point_count; i++) {
			randvec[i] = unit_vector(vec3::random(interval(-1, 1)));
		}

		perlin_generate_perm(perm_x);
		perlin_generate_perm(perm_y);
		perlin_generate_perm(perm_z);
	}

	double noise(const point3& p) const {
		// Linear interpolation to smooth randomness
		double u = p.x() - std::floor(p.x());
		double v = p.y() - std::floor(p.y());
		double w = p.z() - std::floor(p.z());

		auto i = int(std::floor(p.x())) & 255;
		auto j = int(std::floor(p.y())) & 255;
		auto k = int(std::floor(p.z())) & 255;
		vec3 c[2][2][2];

		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = randvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

		return perlin_interp(c, u, v, w);
	}

	double turb(const point3& p, int depth) const {
		auto accum = 0.0;
		auto temp_p = p;
		auto weight = 1.0;

		for (int i = 0; i < depth; i++) {
			accum += weight * noise(temp_p);
			weight *= 0.5;
			temp_p *= 2;
		}

		return std::fabs(accum);
	}
private:
	static const int point_count = 256;
	vec3 randvec[point_count];
	double randfloat[point_count];
	int perm_x[point_count], perm_y[point_count], perm_z[point_count];

	static void perlin_generate_perm(int* p) {
		for (int i = 0; i < point_count; i++)
			p[i] = i;

		permute(p, point_count);
	}

	static void permute(int* p, int n) {
		for (int i = n - 1; i > 0; i--) {
			int target = random_int(0, i);
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w) {
		auto uu = u * u * (3 - 2 * u);
		auto vv = v * v * (3 - 2 * v);
		auto ww = w * w * (3 - 2 * w);
		auto accum = 0.0;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++) {
					vec3 weight_v(u - i, v - j, w - k);
					accum += (i * uu + (1 - i) * (1 - uu)) * (j * vv + (1 - j) * (1 - vv)) * (k * ww + (1 - k) * (1 - ww)) * dot(c[i][j][k], weight_v);
				}

		return accum;
	}
};
#endif
