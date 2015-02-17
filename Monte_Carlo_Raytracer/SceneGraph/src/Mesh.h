#ifndef MESH_H 
#define MESH_H 

#include "Geometry.h"

class Mesh : public Geometry
{
public:
    Mesh();
    virtual ~Mesh();

    virtual void buildGeomtery();
	virtual void add_vertex(float v[]);
	virtual void add_normal(float v[]); 
	virtual void add_texture(float v[]); 
	virtual void add_indice(int vi[]); 
	void display();
	
	Intersection triangle_intersect_basic(glm::mat4 const& T, const Ray &ray, glm::vec3 const& p0, 
		glm::vec3 const& p1, glm::vec3 const& p2, bool is_ray_local) const; 
	
	inline Intersection triangle_intersect(const Ray &ray, int normal_index) const; 

	bool backface_cull(const glm::vec3 raydir, int normal_index) const;

protected:
    /// Compute an intersection with an OBJECT-LOCAL-space ray.
    virtual Intersection intersectImpl(const Ray &ray) const; 
	bool check_bv(const Ray &ray) const;


};

#endif
