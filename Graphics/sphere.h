#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
public:
	sphere(const point3& center, double radius, shared_ptr<material> mat) : center(center), radius(std::fmax(0, radius)), mat(mat) {}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		// Vector from origin of pov to center of the sphere 
		vec3 oc = center - r.origin();

		// Calculating a,b,c to solve quadratic formula
		double a = r.direction().length_squared();
		double h = dot(r.direction(), oc);
		double c = oc.length_squared() - radius * radius;
		double discriminant = h * h - a * c;

		if (discriminant < 0)
			return false;

		double sqrt_discriminant = std::sqrt(discriminant);

		// Solving quadratic formula and saving the nearest root in range

		// Test negative first
		double root = (h - sqrt_discriminant) / a;
		if (!ray_t.surrounds(root)) {
			// If no solution using negative, test positive
			root = (h + sqrt_discriminant) / a;
			if (!ray_t.surrounds(root)) {
				// No solution
				return false;
			}
		}

		// Saving hit record
		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		rec.mat = mat;

		return true;
	}
private:
	point3 center;
	double radius;
	shared_ptr<material> mat;
};
#endif;