#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include "rtweekend.h" // Need to remove after implementation
#include <mutex>
#include <vector>
#include <thread>
#include <iostream>

class camera {
public:
    double      aspect_ratio         = 1.0;  // Ratio of image width over height
    int         image_width          = 100;  // Rendered image width in pixel count
    int         samples_per_pixel    = 10;   // Count of random samples for each pixel, performance heavy
    uint16_t    max_depth            = 10;   // Maximum number of ray bounces into scenes


    double vfov     = 90;               // Vertical fov
    point3 lookfrom = point3(0, 0, 0);	// Camera position
    point3 lookat   = point3(0, 0, -1);	// Camera target
    vec3   vup      = vec3(0, 1, 0);	// Camera-relative "up" direction

    // Defocus (DOF)
    double defocus_angle = 0; // Higher angle = more blur, the rays are allowed to spread more or less
    double focus_dist    = 10;// How far away is the focus plane from lookfrom 

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        int num_threads = std::thread::hardware_concurrency(); // Get available CPU threads
        int rows_per_thread = std::max(1, image_height / num_threads);


        std::vector<std::thread> threads;
        std::vector<color> pixel_data(image_width * image_height);

        for (int t = 0; t < num_threads; t++) {
            int start_y = t * rows_per_thread;
            int end_y = (t == num_threads - 1) ? image_height : (t + 1) * rows_per_thread;

            std::clog << "Dispatching " << start_y << " to " << end_y << " lines to a thread\n";
            threads.emplace_back([this, &pixel_data, &world, start_y, end_y]() {
                this->render_section(pixel_data, world, start_y, end_y);
            });

        }

        for (auto& thread : threads) {
            thread.join();
        }

        for (int j = 0; j < image_height; j++) {
            for (int i = 0; i < image_width; i++) {
                write_color(std::cout, pixel_data[j * image_width + i]);
            }
        }

        std::clog << "\rDone.                 \n";
    }

    void render_section(std::vector<color>& pixel_data, const hittable& world, int start_y, int end_y) {
        for (int j = start_y; j < end_y; j++) {
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }

                pixel_data[j * image_width + i] = pixel_samples_scale * pixel_color;
            }
            std::clog << "Thread " << std::this_thread::get_id() <<  " wrote line " << j << "\n";
        }
    }

private:
    int    image_height;        // Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    point3 center;              // Camera center
    point3 pixel00_loc;         // Location of pixel 0,0
    vec3   pixel_delta_u;       // Offset to pixel to the right
    vec3   pixel_delta_v;       // Offset to pixel below
    vec3   u, v, w;             // Camera frame basis vectors (allows us to have a orthogonal reference)
    vec3   defocus_disk_u;      // Defocus disk horizontal radius
    vec3   defocus_disk_v;      // Defocus disk vertical radius

    std::vector<color> pixel_data;

	void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta / 2);
        double viewport_height = 2.0 * h * focus_dist;
        double viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Vectors allowing to move from one pixel to another adjacent one
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate location of the starting point of the image (upper left pixel)
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        double defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
	}

    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.
        
        vec3 offset = sample_square();
        point3 pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        point3 ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        double ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        vec3 p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

	color ray_color(const ray& r, uint16_t depth,const hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0, 0, 0);

		hit_record rec;
		if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
		}

		vec3 unit_direction = unit_vector(r.direction());

		// Using one value between 0 (white) and 1 (blue), we can get all rgb values between blue and white 
		double a = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
	}
};
#endif // ! CAMERA_H