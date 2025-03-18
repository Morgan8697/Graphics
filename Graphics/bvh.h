#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

/*
 * Bounding Volume Hierarchy Node (BVH)
 * ------------------------------------
 * Represents a node in a BVH tree for accelerating ray-object intersection tests.
 *
 * Purpose:
 * --------
 * Optimizes ray tracing by hierarchically grouping objects with bounding boxes (AABBs),
 * reducing the number of intersection tests needed. This significantly speeds up rendering.
 *
 * Performance:
 * ------------
 * - **Best Case (Balanced Tree):** O(log N) traversal, checking only a small subset of objects.
 * - **Average Case:** Typically near O(log N) with good splits (e.g., along longest axis).
 * - **Worst Case:** O(N) if the BVH is degenerate (e.g., unbalanced) or if the ray hits every AABB.
 *
 * Usage:
 * ------
 * 1. Build BVH: Automatically constructs a tree by recursively subdividing objects.
 * 2. Hit Test: Efficiently checks ray intersections, prioritizing early exits.
 *
 * Key Features:
 * -------------
 * - Sorts objects along the longest axis for balanced partitions.
 * - Fast AABB-ray intersection checks (slab method).
 * - Supports merging bounding boxes for internal nodes.
 *
 * Practical Impact:
 * -----------------
 * Dramatically reduces render time for scenes with many objects.
 * In pathological cases, performance degrades gracefully to brute-force levels.
 */



class bvh_node : public hittable {
public:
	bvh_node(hittable_list list) : bvh_node(list.hittables, 0, list.hittables.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy. 
        // https://raytracing.github.io/books/RayTracingTheNextWeek.html#boundingvolumehierarchies/thebvhnodeclass
	}

    bvh_node(std::vector<shared_ptr<hittable>>& hittables, size_t start, size_t end) {
        bbox = aabb::empty;
        for (size_t hittable_index = start; hittable_index < end; hittable_index++)
            bbox = aabb(bbox, hittables[hittable_index]->bounding_box());

        int axis = bbox.longest_axis();

        auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            left = right = hittables[start];
        }
        else if (object_span == 2) {
            left = hittables[start];
            right = hittables[start + 1];
        }
        else {
            std::sort(std::begin(hittables) + start, std::begin(hittables) + end, comparator);

            size_t mid = start + object_span / 2;
            left = make_shared<bvh_node>(hittables, start, mid);
            right = make_shared<bvh_node>(hittables, mid, end);
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t))
            return false;

        // Will stop once the object is an actual hittable and not a bvh_node.
        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }
private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

    static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
        const interval a_axis_interval = a->bounding_box().axis_interval(axis_index);
        const interval b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }
};
#endif
