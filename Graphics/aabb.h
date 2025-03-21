#ifndef AABB_H
#define AABB_H

#include "helper.h"

/*
 * Axis-Aligned Bounding Box (AABB)
 * --------------------------------
 * Represents a rectangular 3D volume aligned to the X, Y, Z axes.
 * Used for efficient spatial queries (e.g., ray intersection tests)
 *
 * Key Features:
 * - Construct from intervals or two 3D points (auto-ordering min/max).
 * - Merge two AABBs to get a bounding box that encloses both.
 * - Fast AABB-ray intersection using the slab method (returns true if intersected).
 * - Identify the longest axis to optimize bounding volume hierarchies (BVH).
 * - Provides static constants for an empty box and a universal bounding box.
 *
 * Usage Example:
 * ---------------
 * aabb box(p1, p2);               // Create AABB from two points
 * if (box.hit(ray, t_range)) {    // Fast intersection test
 *     // Ray intersects the box
 * }
 * int axis = box.longest_axis();  // For BVH splitting
 */


class aabb {
public:
	interval x, y, z;

	aabb() {} // Default AABB where intervals are empty, making the AABB empty too

	aabb(const interval& x, const interval& y, const interval& z) : x(x), y(y), z(z) 
	{
		pad_to_minimums();
	}

	// Construct a AABB based on two extreme points
	aabb(const point3& a, const point3& b) {
		// Treat the two points a and b as extrema for the bounding box, so we don't require a
		// particular minimum/maximum coordinate order.

		x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
		y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
		z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
	
		pad_to_minimums();
	}

	aabb(const aabb& box0, const aabb& box1) {
		x = interval(box0.x, box1.x);
		y = interval(box0.y, box1.y);
		z = interval(box0.z, box1.z);
	}

	const interval& axis_interval(int n) const {
		if (n == 1) return y;
		if (n == 2) return z;
		return x;
	}

	/*
		AABB-ray intersection using the slab method.
		- Computes the entry (`t0`) and exit (`t1`) times along X, Y, and Z axes.
		- Updates `t_min` (latest entry) and `t_max` (earliest exit) to track overlap.
		- If `t_max <= t_min`, the ray misses the box (returns false).
		- If all axes allow overlap, the ray intersects the AABB (returns true).
	*/
	bool hit(const ray& r, interval ray_t) const {
		const point3& ray_orig = r.origin();
		const vec3& ray_dir = r.direction();

		for (int axis = 0; axis < 3; axis++) {
			const interval& ax = axis_interval(axis);
			const double adinv = 1.0 / ray_dir[axis]; // Will get infinity if we ray_dir[axis] == 0, see IEEE floating point rule #754

			auto t0 = (ax.min - ray_orig[axis]) * adinv;
			auto t1 = (ax.max - ray_orig[axis]) * adinv;

			if (t0 < t1) {
				if (t0 > ray_t.min) ray_t.min = t0;
				if (t1 < ray_t.max) ray_t.max = t1;
			}
			else {
				if (t1 > ray_t.min) ray_t.min = t1;
				if (t0 < ray_t.max) ray_t.max = t0;
			}

			if (ray_t.max <= ray_t.min)
				return false;
		}
		return true;
	}

	int longest_axis() const {
		// Returns the index of the longest axis of the bounding box.

		if (x.size() > y.size())
			return x.size() > z.size() ? 0 : 2;
		else
			return y.size() > z.size() ? 1 : 2;
	}
	
	static const aabb empty, universe;
private:
	void pad_to_minimums() {
		// Adjsut the AABB so that no side is narrower than some delta, padding id necessary.

		double delta = 0.0001;
		if (x.size() < delta) x = x.expand(delta);
		if (y.size() < delta) x = y.expand(delta);
		if (z.size() < delta) x = z.expand(delta);
	}
};

const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
	return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

aabb operator+(const vec3& offset, const aabb& bbox) {
	return bbox + offset;
}

#endif
