#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"
#include <cstdio>

class Sphere : public Geometry
{
public:
    Sphere();
    virtual ~Sphere();

    virtual void buildGeomtery();
    virtual void add_indice(int v[])  ;
    virtual void add_normal(float v[])  ;
    virtual void add_vertex(float v[])  ;
    virtual void add_texture(float v[])  ;

private:
    glm::vec3 center_;
    float radius_;

protected:
    /// Compute an intersection with an OBJECT-LOCAL-space ray.
    virtual Intersection intersectImpl(const Ray &ray) const;
};

#endif