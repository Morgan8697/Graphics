#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"

class material;

struct hit_record {
public:
	point3 p;
	vec3 normal;
	shared_ptr<material> mat;
	double t;
	double u, v; // Texture parameters
	bool front_face;

	void set_face_normal(const ray& r, const vec3& outward_normal) {
		// Sets the hit record normal vector.
		// NOTE: We assume outward_normal to be of unit length, we could reunit it, but it would add calculation time.

		// If the ray and the normal are in the same direction, it means the ray is coming out of the hittable object, if its in the same direction, it means the ray is entering the object
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable {
public:
	virtual ~hittable() = default;
	virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
	virtual aabb bounding_box() const = 0;
};
#endif