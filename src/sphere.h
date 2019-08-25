#ifndef SPHEREH
#define SPHEREH
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

#include "hitable.h"


class sphere: public hitable  {
    public:
        sphere() {}
        sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m)  {};
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        vec3 center;
        float radius;
        material *mat_ptr;
};


bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {

	/*
		@author: Vassillen Chizhov, August 2019

		Addresses issues: #17, #54


		Sphere intersection derivation:

		Canonic sphere equation:

		||p-center||^2 = radius^2

		Parameteric ray equation:

		r(t) = r.origin() + t * r.direction()

		Plug in p = r(t) and solve for t:

		dot(r.origin() + t * r.direction() - center, r.origin() + t * r.direction() - center) = radius * radius
		Let oc = r.origin() - center

		Use the linearity, distributivity and commutativity of the dot product to get:

		dot(oc,oc) - radius * radius + 2 * dot(r.direction(),oc) * t + dot(r.direction(),r.direction()) * t * t = 0

		Let A = dot(r.direction(),r.direction()), B = dot(r.direction(),oc), C = dot(oc,oc) - radius * radius, then:

		A * t^2 + 2 * B * t + C = 0

		D' = B^2 - A * C
		D = 4 * B^2 - 4 * A * C = 4 * (B^2 - A * C) = 4 * D'

		t_1 = (-2*B - sqrt(D))/(2*A) = (-2*B - sqrt(4*D'))/(2*A) = 2 * (-B - sqrt(D'))/(2*A) = (-B - sqrt(D'))/A

		Analogously:

		t_2 = (-2*B + sqrt(D))/(2*A) = (-2*B + 2*sqrt(D'))/(2*A) = (-B + sqrt(D'))/A

		(Additionally the minus in front of B can be gotten rid of if one uses oc = center-r.origin(), and the division 
		if the rays are normalized, since then length(r.direction()) = 1, however I did not dare change those since 
		it will cause a discrepancy with the book).
	*/

    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - a*c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }

		/*
			Adresses issue: #15

			The t_2 = (-b + sqrt(discriminant)) / a; 
			is necessary if the ray starts inside the sphere, or if the first intersection 
			got culled due to temp <= t_min
		*/
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}


#endif
