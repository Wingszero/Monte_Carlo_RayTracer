#ifndef CYLINDER_H
#define CYLINDER_H

#include "Geometry.h"
#include<iostream>


class Cylinder : public Geometry
{
public:
    Cylinder();
	//Cylinder(glm::vec3 center, float r, float h); 
    virtual ~Cylinder();

    virtual void buildGeomtery();
    virtual void add_indice(int v[]);
    virtual void add_normal(float v[]);
    virtual void add_vertex(float v[]);
    virtual void add_texture(float v[]);

	float test_intersect_cap(const Ray &ray) const;

private:
    glm::vec3 center_;
    float radius_;
    float height_;

protected:
    /// Compute an intersection with an OBJECT-LOCAL-space ray.
    virtual Intersection intersectImpl(const Ray &ray) const;
};

#endif