#ifndef MATERIALH
#define MATERIALH
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

#include "ray.h"
#include "hitable.h"
//#include "random.h"
#include "rng.hpp" // random() provides better random numbers than rand() % (RAND_MAX + 1.0);

struct hit_record;


float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}


bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
    vec3 uv = unit_vector(v);
    float dt = dot(uv, n);
    float discriminant = 1.0f - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
    else
        return false;
}


vec3 reflect(const vec3& v, const vec3& n) {
     return v - 2*dot(v,n)*n;
}


vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p = 2.0f*vec3(random(),random(),random()) - vec3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}

/*
	Addresses issue: #26

	Using random_on_unit_sphere() in the lambertian material doesn't generate cosine weighted hemisphere samples, 
	but rather cos^3 weighted hemisphere samples, resulting in a cos^2 brdf and not a diffuse one (the two differ 
	only slightly visually though).

	Note that this has a singularity at (0,0,0), theoretically the probability of 
	picking (0,0,0) is 0 (since a point is a set of measure 0 with respect to the volume Lebesgue measure, 
	however in practice with finite precision this is not so).
*/
vec3 random_on_unit_sphere()
{
	vec3 res = random_in_unit_sphere();
	res.make_unit_vector();
	return res;
}


class material  {
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};


class lambertian : public material {
    public:
        lambertian(const vec3& a) : albedo(a) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
			/*
				Addresses issue: #9

				Fixes scattering on the outside, by picking the correct normal. 
				And adds an offset along the normal to avoid self-intersection caused by numerical errors.
			*/

			// always pick the correct facing normal (in case of scattering inside a sphere)
			vec3 normal = dot(rec.normal, r_in.direction()) < 0.0f ? rec.normal : -rec.normal;

			// use correct normal
             vec3 target = rec.p + normal + random_on_unit_sphere(); // random_in_unite_sphere() -> random_on_unit_sphere(), Addresses issue: #26
             
			 // offset the origin to avoid self-intersection
			 scattered = ray(rec.p + normal * OFFSET_EPSILON, target-rec.p);
             attenuation = albedo;
             return true;
        }

        vec3 albedo;
};


class metal : public material {
    public:
        metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
			/*
				Addresses issue: #9

				Fixes scattering on the outside, by picking the correct normal.
				And adds an offset along the normal to avoid self-intersection caused by numerical errors.
			*/

			// always pick the correct facing normal (in case of scattering inside a sphere)
			vec3 normal = dot(rec.normal, r_in.direction()) < 0.0f ? rec.normal : -rec.normal;
			// use correct normal
            vec3 reflected = reflect(unit_vector(r_in.direction()), normal);
			// offset the origin to avoid self-intersection
            scattered = ray(rec.p + normal * OFFSET_EPSILON, reflected + fuzz*random_on_unit_sphere()); // on_unit_sphere for lambertian behaviour
            attenuation = albedo;

			// use the correct normal
            return (dot(scattered.direction(), normal) > 0);
        }
        vec3 albedo;
        float fuzz;
};


class dielectric : public material {
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
			/*
				Addresses issue: #9

				Adds an offset along the normal to avoid self-intersection caused by numerical errors.
			*/

             vec3 outward_normal;
             vec3 reflected = reflect(r_in.direction(), rec.normal);
             float ni_over_nt;
             attenuation = vec3(1.0, 1.0, 1.0);
             vec3 refracted;
             float reflect_prob;
             float cosine;
             if (dot(r_in.direction(), rec.normal) > 0) {
                  outward_normal = -rec.normal;
                  ni_over_nt = ref_idx;
               // cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
             }
             else {
                  outward_normal = rec.normal;
                  ni_over_nt = 1.0f / ref_idx;
                  cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
             }
			 if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			 {
				 /*
					Addresses issue: #9

					Comment: schlick(cosine, ref_idx) may be confusing, since one would expect schlick(cosine, ni_over_nt)
					But as it happens those two are equal.
				 */
				 reflect_prob = schlick(cosine, ref_idx);
			 }
             else
                reflect_prob = 1.0;

			 // offset the origin to avoid self-intersection
             if (random() < reflect_prob)
                scattered = ray(rec.p + outward_normal * OFFSET_EPSILON, reflected);
             else
                scattered = ray(rec.p - outward_normal * OFFSET_EPSILON, refracted); // negative offset, since the origin must continue on the other side of the surface

             return true;
        }

        float ref_idx;
};


#endif
