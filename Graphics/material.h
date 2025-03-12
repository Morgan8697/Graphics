#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"

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
	lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
	lambertian(shared_ptr<texture> tex) : tex(tex) {};

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 scatter_direction = rec.normal + random_unit_vector();

		// normal + random_unit_vector could be opposites and the resultant would be 0, in this case we will discard the random_unit_vector
		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction, r_in.time());
		attenuation = tex->value(rec.u, rec.v, rec.p);
		return true;
	}

private:
	shared_ptr<texture> tex;
};

class metal : public material {
public:
	// Represent perfect reflexion since its smooth
	metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
		scattered = ray(rec.p, reflected, r_in.time());
		attenuation = albedo;
		// true if scattered and normal are on the same hemisphere
		return (dot(scattered.direction(), rec.normal) > 0);
	}

private:
	// How much the reflection is perfect or not
	color albedo;

	// How much the reflected rays are potentially reoriented after reflection
	double fuzz;
};

class dielectric : public material {
public:
	dielectric(double refraction_index) : refraction_index(refraction_index) {}

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		attenuation = color(1.0, 1.0, 1.0);
		double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;
		
		vec3 unit_direction = unit_vector(r_in.direction());

		double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

		bool can_refract = ri * sin_theta <= 1.0;
		vec3 direction;

		if (can_refract || reflectance(cos_theta, ri) > random_double())
			direction = refract(unit_direction, rec.normal, ri);
		else
			direction = reflect(unit_direction, rec.normal); // Total Internal Reflection

		scattered = ray(rec.p, direction, r_in.time());
		return true;
	}

private:
	// Refractive index in vacuum or air, or the ratio of the material's refractive index over the refractive index of the enclosing media
	double refraction_index;

	// Schlick's approximation, a simple equation that approximate a way more complex equation to calculate reflection depending on angle https://en.wikipedia.org/wiki/Schlick%27s_approximation
	static double reflectance(double cosine, double refraction_index) {
		double r0 = (1 - refraction_index) / (1 + refraction_index);
		r0 = r0 * r0;
		return r0 + (1 - r0) * std::pow((1 - cosine), 5);
	}
};
#endif
