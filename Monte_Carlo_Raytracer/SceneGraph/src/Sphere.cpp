#include "Sphere.h"

#ifndef INTER_EPS
#define INTER_EPS 1e-3f
#endif

// Creates a unit sphere.
Sphere::Sphere() :
    Geometry(SPHERE),
    center_(glm::vec3(0.f, 0.f, 0.f)),
    radius_(1.0f)
{
    buildGeomtery();
}

Sphere::~Sphere() {}

void Sphere::buildGeomtery()
{
    vertices_.clear();
    colors_.clear();
    normals_.clear();
    indices_.clear();

    // Find vertex positions for the sphere.
    unsigned int subdiv_axis = 16;      // vertical slices
    unsigned int subdiv_height = 16;        // horizontal slices
    float dphi = PI / subdiv_height;
    float dtheta = 2.0f * PI / subdiv_axis;
    float epsilon = 0.0001f;
    glm::vec3 color (0.6f, 0.6f, 0.6f);

    // North pole
    glm::vec3 point (0.0f, 1.0f, 0.0f);
    normals_.push_back(point);
    // scale by radius_ and translate by center_
    vertices_.push_back(center_ + radius_ * point);

    for (float phi = dphi; phi < PI; phi += dphi) {
        for (float theta = dtheta; theta <= 2.0f * PI + epsilon; theta += dtheta) {
            float sin_phi = sin(phi);

            point[0] = sin_phi * sin(theta);
            point[1] = cos(phi);
            point[2] = sin_phi * cos(theta);

            normals_.push_back(point);
            vertices_.push_back(center_ + radius_ * point);
        }
    }
    // South pole
    point = glm::vec3(0.0f, -1.0f, 0.0f);
    normals_.push_back(point);
    vertices_.push_back(center_ + radius_ * point);

    // fill in index array.
    // top cap
    for (unsigned int i = 0; i < subdiv_axis - 1; ++i) {
        indices_.push_back(0);
        indices_.push_back(i + 1);
        indices_.push_back(i + 2);
    }
    indices_.push_back(0);
    indices_.push_back(subdiv_axis);
    indices_.push_back(1);

    // middle subdivs
    unsigned int index = 1;
    for (unsigned int i = 0; i < subdiv_height - 2; i++) {
        for (unsigned int j = 0; j < subdiv_axis - 1; j++) {
            // first triangle
            indices_.push_back(index);
            indices_.push_back(index + subdiv_axis);
            indices_.push_back(index + subdiv_axis + 1);

            // second triangle
            indices_.push_back(index);
            indices_.push_back(index + subdiv_axis + 1);
            indices_.push_back(index + 1);

            index++;
        }
        // reuse vertices from start and end point of subdiv_axis slice
        indices_.push_back(index);
        indices_.push_back(index + subdiv_axis);
        indices_.push_back(index + 1);

        indices_.push_back(index);
        indices_.push_back(index + 1);
        indices_.push_back(index + 1 - subdiv_axis);

        index++;
    }

    // end cap
    unsigned int bottom = (subdiv_height - 1) * subdiv_axis + 1;
    unsigned int offset = bottom - subdiv_axis;
    for (unsigned int i = 0; i < subdiv_axis - 1 ; ++i) {
        indices_.push_back(bottom);
        indices_.push_back(i + offset);
        indices_.push_back(i + offset + 1);
    }
    indices_.push_back(bottom);
    indices_.push_back(bottom - 1);
    indices_.push_back(offset);

    // colors
    for (unsigned int i = 0; i < vertices_.size(); ++i) {
        colors_.push_back(glm::vec3(0.6, 0.6, 0.6));
    }
}
void Sphere::add_indice(int v[])
{

}
void Sphere::add_normal(float v[])
{

}
void Sphere::add_vertex(float v[])
{

}
void Sphere::add_texture(float v[])
{

}

Intersection Sphere::intersectImpl(const Ray &ray) const
{
	Intersection false_res;
	false_res.t = -1;
	Intersection res;

	float dis = glm::distance(ray.orig, center_);
	bool inside = (dis <= radius_ + EPS);

	glm::vec3 DIR = glm::normalize(ray.dir);	
	
	glm::vec3 L = (center_ - ray.orig);
	float tca = glm::dot(L, DIR);
    if (!inside && tca < 0.f) 
	{
		return false_res;
	}

	float radius2 = radius_ * radius_;
	float d2 = glm::dot(L,L) - tca * tca;
    if (d2 > radius2) //out of the tangent
	{
		return false_res;
	}

	float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

	float t;
	t = t0;
	//if(t <= -INTER_EPS) //the before intersection point
	if(t < INTER_EPS) //the before intersection point
		t = t1;
	glm::vec3 pos = ray.orig + DIR * t;
	res.t = t;
	res.normal = pos - center_; 
	res.pos = pos;
	//printf("t0: %f t1: %f \n", t0, t1);
	return res; 
}
