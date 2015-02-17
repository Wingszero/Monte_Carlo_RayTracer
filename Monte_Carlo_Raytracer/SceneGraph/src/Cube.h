#ifndef CUBE_H 
#define CUBE_H 

#include "Geometry.h"

class Cube : public Geometry
{
public:
    Cube();
    virtual ~Cube();

    virtual void buildGeomtery();
    virtual void add_indice(int v[])  ;
    virtual void add_vertex(float v[]) ;
    virtual void add_normal(float v[]) ;
    virtual void add_texture(float v[]);

private:
    glm::vec3 center_;
    float length_; 
	glm::vec3 min_pt, max_pt;

protected:
    /// Compute an intersection with an OBJECT-LOCAL-space ray.
    virtual Intersection intersectImpl(const Ray &ray) const;
};

#endif
