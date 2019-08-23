//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <iostream>
#include <limits>				// std::numeric_limits<float>::infinity()
#include <fstream>				// doesn't require output redirection
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "rng.hpp"				// random()



vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001f, std::numeric_limits<float>::infinity(), rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5f*(unit_direction.y() + 1.0f);
        return (1.0f-t)*vec3(1.0f, 1.0f, 1.0f) + t*vec3(0.5f, 0.7f, 1.0f);
    }
}


hitable *random_scene() {
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random();
            vec3 center(a+0.9f*random(),0.2f,b+0.9f*random());
            if ((center-vec3(4,0.2f,0)).length() > 0.9f) {
                if (choose_mat < 0.8f) {  // diffuse
                    list[i++] = new sphere(
                        center, 0.2f,
                        new lambertian(vec3(random()*random(),
                                            random()*random(),
                                            random()*random()))
                    );
                }
                else if (choose_mat < 0.95f) { // metal
                    list[i++] = new sphere(
                        center, 0.2f,
                        new metal(vec3(0.5f*(1 + random()),
                                       0.5f*(1 + random()),
                                       0.5f*(1 + random())),
                                  0.5f*random())
                    );
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0f, new dielectric(1.5f));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0f, new lambertian(vec3(0.4f, 0.2f, 0.1f)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0f, new metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

    return new hitable_list(list,i);
}


int main() {

	// faster preview
	/*
		int nx = 1200;
		int ny = 800;
	*/
	int nx = 320;
	int ny = 240;
    int ns = 10;


	/*
		Addresses issue: #18
		std::ofstream rather than std::cout, so one doesn't need to redirect the output
	*/
	// open output file
	std::ofstream file;
	const char* outputFilename = "output_image.ppm";
	file.open(outputFilename);
	if (!file.good()) std::cerr << "Failed to open file " << outputFilename << std::endl;
    file << "P3\n" << nx << " " << ny << "\n255\n";


    hitable *world = random_scene();

    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0f;
    float aperture = 0.1f;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + random()) / float(nx);
                float v = float(j + random()) / float(ny);
                ray r = cam.get_ray(u, v);
                col += color(r, world,0);
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            file << ir << " " << ig << " " << ib << "\n";
        }
    }

	//close output file
	file.close();

	return 0;
}
