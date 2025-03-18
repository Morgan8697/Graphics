#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"

/*
 * Hittable Interface and Hit Record
 * ---------------------------------
 * Defines an interface for objects that can be intersected by rays, and a data structure to
 * store information about ray-object intersections.
 *
 * Purpose:
 * --------
 * - `hittable`: Abstract base class for any object that can be hit by a ray.
 * - `hit_record`: Stores intersection details such as position, normal, material, and texture coordinates.
 *
 * Hit Record:
 * -----------
 * - Stores hit point, surface normal, texture coordinates (u, v), and ray parameter `t`.
 * - Tracks whether the ray hit the front or back face of the surface.
 * - Automatically adjusts normal direction to oppose incoming ray using `set_face_normal()`.
 *
 * Hittable Interface:
 * -------------------
 * - `hit(ray, t_range, rec)`: Determines if ray hits object within `t_range`, fills `rec`.
 * - `bounding_box()`: Returns an axis-aligned bounding box (AABB) for the object.
 *
 * Usage:
 * ------
 * class sphere : public hittable {
 *     bool hit(const ray& r, interval t_range, hit_record& rec) const override;
 *     aabb bounding_box() const override;
 * };
 */

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