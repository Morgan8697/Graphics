#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material {
public:
	virtual ~material() = default;

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const {
		return false;
	}
};

class lambertian : public material {
public:
	lambertian(const color& albedo) : albedo(albedo) {}

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 scatter_direction = rec.normal + random_unit_vector();

		// normal + random_unit_vector could be opposites and the resultant would be 0, in this case we will discard the random_unit_vector
		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}

private:
	color albedo;
};

class metal : public material {
public:
	// Represent perfect reflexion since its smooth
	metal(const color& albedo) : albedo(albedo) {}

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return true;
	}

private:
	color albedo;
};
#endif
