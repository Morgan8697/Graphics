#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "aabb.h"
#include "hittable.h"

#include <vector>

/*
 * Hittable List
 * -------------
 * Represents a collection of hittable objects, treated as a single composite object.
 * Used to manage and perform ray intersection tests across multiple scene objects.
 *
 * Purpose:
 * --------
 * - Groups multiple hittable objects for organized ray intersection handling.
 * - Computes the bounding box for the entire collection to accelerate ray tracing.
 *
 * Functionality:
 * --------------
 * - `add(hittable)`: Adds an object and updates the bounding box.
 * - `clear()`: Removes all objects from the list.
 * - `hit(ray, t_range, rec)`: Iterates through objects, records closest intersection.
 * - `bounding_box()`: Returns the bounding box enclosing all objects in the list.
 *
 * Performance:
 * ------------
 * - Performs linear search over objects for ray hits.
 * - Used as a base for more optimized structures like BVH.
 *
 * Usage:
 * ------
 * hittable_list scene;
 * scene.add(make_shared<sphere>(...));
 * if (scene.hit(ray, t_range, hit_rec)) {
 *     // Process intersection
 * }
 */

class hittable_list : public hittable {
public:
	std::vector<shared_ptr<hittable>> hittables;

	hittable_list() {}
	hittable_list(shared_ptr<hittable> hittable_object) { add(hittable_object); }

	void clear() { hittables.clear(); }

	void add(shared_ptr<hittable> hittable_object) {
		hittables.push_back(hittable_object);
		bbox = aabb(bbox, hittable_object->bounding_box());
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		hit_record temp_rec;
		bool hit_anything = false;
		double closest_so_far = ray_t.max;

		for (const shared_ptr<hittable>& hittable_object : hittables) {
			if (hittable_object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return hit_anything;
	}
	aabb bounding_box() const override { return bbox; }

private:
	aabb bbox;
};
#endif