#ifndef VEC3_H
#define VEC3_H

#include "helper.h"

class vec3
{
public:
	double e[3];

	vec3() : e{0,0,0} {}
	vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }

	vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	vec3& operator +=(const vec3& v) {
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return *this;
	}

	vec3& operator*=(double t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	vec3& operator/=(double t) {
		return *this *= 1 / t;
	}

	double length() const {
		return sqrt(length_squared());
	}

	double length_squared() const {
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

	bool near_zero() const {
		// Return true if the vector is close to zero in all dimensions
		double near_zero_value = 1e-18;
		return (std::fabs(e[0]) < near_zero_value) && (std::fabs(e[1]) < near_zero_value) && (std::fabs(e[2]) < near_zero_value);
	}

	static vec3 random() {
		return vec3(random_double(), random_double(), random_double());
	}

	static vec3 random(interval range) {
		return vec3(random_double(range.min, range.max), random_double(range.min, range.max), random_double(range.min, range.max));
	}
};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
	return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
	return (1/t) * v;
}
inline double dot(const vec3& u, const vec3& v) {
	return u.e[0] * v.e[0]
		+ u.e[1] * v.e[1]
		+ u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
	return v / v.length();
}

inline vec3 random_in_unit_disk() {
	while (true) {
		vec3 p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() < 1)
			return p;
	}
}

inline vec3 random_unit_vector() {
	while (true) {
		point3 p = vec3::random(interval(-1, 1));
		double len_sqr = p.length_squared();

		// Due to floating precision, a really small value can underflow to zero when squared, we need to ban this location really close to the center.
		if (1e-160 < len_sqr && len_sqr <= 1)
			return p / sqrt(len_sqr);
	}
}

// WARNING: This implementation is simple but costly in the performance see Von Mises Distribution for potential improvement.
inline vec3 random_on_hemisphere(const vec3& normal) {
	vec3 on_unit_sphere = random_unit_vector();
	if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
		return on_unit_sphere;
	else
		return -on_unit_sphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n) * n;
}

// uv represents the refracted ray, n the normal and etai_over_etat ni/nt
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
	// Since uv and n are unit vectors a*b = cos(theta)
	float cos_theta = std::fmin(dot(-uv, n), 1.0);

	// components that are perpendicular to the normal
	vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);

	// components that are parallel to the normal
	vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;

	return r_out_perp + r_out_parallel;
}
#endif // !

