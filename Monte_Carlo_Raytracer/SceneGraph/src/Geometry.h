#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Ray.h"
#include "bv.h"
#include "kd_tree.h"
#include "Intersection.h"

using namespace std;


#define PI 3.14159265

#ifndef EPS
#define EPS 1e-6f
#endif

const float Radian = float(PI)/ 180.0f;

// An abstract base class for geometry in the scene graph.
class Geometry
{
public:
    // Enums for the types of geometry that your scene graph is required to contain.
    // Feel free to add more.
    enum geometryType {CUBE, SPHERE, CYLINDER, MESH};
    explicit Geometry(geometryType);
    virtual ~Geometry();

    // Function for building vertex data, i.e. vertices, colors, normals, indices.
    // Implemented in Sphere and Cylinder.
    virtual void buildGeomtery() = 0;

    virtual void add_vertex(float v[]) = 0;
    virtual void add_normal(float v[]) = 0;
    virtual void add_texture(float v[])= 0;
    virtual void add_indice(int v[])  = 0;

	void init_normals(); //vertex normals
    void display() ;
	
	//bounding volume
	void init_bv();
	BV *bv; 

	//kdtree
	KdTree* kd_tree;
	void build_kdtree(int); 
	
	//triangle plance normals
	void init_tri_normals(); 
	vector<glm::vec3>tri_normals;

    // Getters
    const vector<glm::vec3>& getVertices() const
    {
        return vertices_;
    };
    const vector<glm::vec3>& getNormals() const
    {
        return normals_;
    };
    const vector<glm::vec3>& getColors() const
    {
        return colors_;
    };
    const vector<unsigned int>& getIndices() const
    {
        return indices_;
    };
    
	const vector<glm::vec2>& getTexture() const
    {
        return texture_;
    };


    unsigned int getVertexCount() const
    {
        return vertices_.size();
    };
    unsigned int getIndexCount() const
    {
        return indices_.size();
    };
    unsigned int getTextureCount() const
    {
        return texture_.size();
    };

    const geometryType getGeometryType() const
    {
        return type_;
    };
    
	void init_colors(float in_colors[]) 
    {
		colors_.clear();
		//printf("in_colors: %f %f %f\n", in_colors[0], in_colors[1], in_colors[2]);
		memcpy(self_colors, in_colors, sizeof(float) * 3);
		glm::vec3 color(self_colors[0], self_colors[1], self_colors[2]); 
		for (unsigned int i = 0; i < vertices_.size(); ++i) 
		{
			colors_.push_back(color);
		}
	}
	
	void set_colors(float in_colors[]) 
    {
		colors_.clear();
		glm::vec3 color(in_colors[0], in_colors[1], in_colors[2]); 
		for (unsigned int i = 0; i < vertices_.size(); ++i) 
		{
			colors_.push_back(color);
		}
	}
	
	void recover_colors()
    {
		colors_.clear();
		glm::vec3 color(self_colors[0], self_colors[1], self_colors[2]); 
		for (unsigned int i = 0; i < vertices_.size(); ++i) 
		{
			colors_.push_back(color);
		}
	}

	bool is_cube()
	{
		return type_ == CUBE;
	}

	bool is_sphere()
	{
		return type_ == SPHERE;
	}

    /// Compute an intersection with a WORLD-space ray.
    Intersection intersect(const glm::mat4 &T, Ray ray_world) const;

protected:
    geometryType type_;

    vector<glm::vec3> vertices_;        // vertex buffer
    vector<glm::vec3> normals_;         // normal buffer
    vector<glm::vec3> colors_;          // color buffer
    vector<unsigned int> indices_;      // index buffer
    vector<glm::vec2> texture_;      //texture buffer 
	float self_colors[4];

    /// Compute an intersection with an OBJECT-LOCAL-space ray.
    virtual Intersection intersectImpl(const Ray &ray) const = 0;
};

#endif
