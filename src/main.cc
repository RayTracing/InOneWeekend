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

#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"


vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, MAXFLOAT, rec)) { 
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
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}


hitable *random_scene() {
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a+0.9*drand48(),0.2,b+0.9*drand48()); 
            if ((center-vec3(4,0.2,0)).length() > 0.9) { 
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new sphere(center, 0.2, new lambertian(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(center, 0.2,
                            new metal(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())),  0.5*drand48()));
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new hitable_list(list,i);
}


void write_image(uint8_t *image, int nx, int ny) {
  std::cout << "P3\n" << nx << " " << ny << "\n255\n";
  for (int j = ny-1; j >= 0; j--) {
    for (int i = 0; i < nx; ++i) {
      std::cout << static_cast<int>(image[3*(j*nx+i)+0]) << " ";
      std::cout << static_cast<int>(image[3*(j*nx+i)+1]) << " ";
      std::cout << static_cast<int>(image[3*(j*nx+i)+2]) << std::endl;
    }
  }
}


int main() {
    int nx = 1200;
    int ny = 800;
    int ns = 10;
    hitable *list[5];
    float R = cos(M_PI/4);
    list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list,5);
    world = random_scene();

    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    std::vector<uint8_t> image(nx * ny * 3);
    const int k_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(k_threads);
    for (int t = 0; t < k_threads; ++t) {
        threads[t] = std::thread(std::bind([&](int start, int end, int t) {
            for (int j = start; j < end; j++) {
                for (int i = 0; i < nx; i++) {
                    vec3 col(0, 0, 0);
                    for (int s=0; s < ns; s++) {
                        float u = float(i + drand48()) / float(nx);
                        float v = float(j + drand48()) / float(ny);
                        ray r = cam.get_ray(u, v);
                        col += color(r, world,0);
                    }
                    col /= float(ns);
                    col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
                  
                    image[3*(j*nx+i)+0] = int(255.99*col[0]);
                    image[3*(j*nx+i)+1] = int(255.99*col[1]);
                    image[3*(j*nx+i)+2] = int(255.99*col[2]);
                }
            }
        }, t*ny/k_threads, (t+1)==k_threads ? ny : (t+1)*ny/k_threads, t));
    }
    for (int t = 0; t < k_threads; ++t) {
        threads[t].join();
    }
    write_image(&image[0], nx, ny);
}
