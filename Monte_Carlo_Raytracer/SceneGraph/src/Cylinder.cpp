#include "Cylinder.h"

// Creates a unit cylinder centered at (0, 0, 0)
Cylinder::Cylinder() :
    Geometry(CYLINDER),
    center_(glm::vec3(0.f, 0.f, 0.f)),
    radius_(0.5f),
    height_(1.0f)
{
    buildGeomtery();
}

Cylinder::~Cylinder() {}

void Cylinder::buildGeomtery()
{
    vertices_.clear();
    colors_.clear();
    normals_.clear();
    indices_.clear();

    unsigned short subdiv = 20;
    float dtheta = 2 * PI / subdiv;

    glm::vec4 point_top(0.0f, 0.5f * height_, radius_, 1.0f),
        point_bottom (0.0f, -0.5f * height_, radius_, 1.0f);
    vector<glm::vec3> cap_top, cap_bottom;

    // top and bottom cap vertices
    for (int i = 0; i < subdiv + 1; ++i) {
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), i * dtheta, glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), center_);

        cap_top.push_back(glm::vec3(translate * rotate * point_top));
        cap_bottom.push_back(glm::vec3(translate * rotate * point_bottom));
    }

    //Create top cap.
    for ( int i = 0; i < subdiv - 2; i++) {
        vertices_.push_back(cap_top[0]);
        vertices_.push_back(cap_top[i + 1]);
        vertices_.push_back(cap_top[i + 2]);
    }
    //Create bottom cap.
    for (int i = 0; i < subdiv - 2; i++) {
        vertices_.push_back(cap_bottom[0]);
        vertices_.push_back(cap_bottom[i + 1]);
        vertices_.push_back(cap_bottom[i + 2]);
    }
    //Create barrel
    for (int i = 0; i < subdiv; i++) {
        //Right-side up triangle
        vertices_.push_back(cap_top[i]);
        vertices_.push_back(cap_bottom[i + 1]);
        vertices_.push_back(cap_bottom[i]);
        //Upside-down triangle
        vertices_.push_back(cap_top[i]);
        vertices_.push_back(cap_top[i + 1]);
        vertices_.push_back(cap_bottom[i + 1]);
    }

    // create normals
    glm::vec3 top_centerpoint(0.0f , 0.5f * height_ , 0.0f),
        bottom_centerpoint(0.0f, -0.5f * height_, 0.0f);
    glm::vec3 normal(0, 1, 0);

    // Create top cap.
    for (int i = 0; i < subdiv - 2; i++) {
        normals_.push_back(normal);
        normals_.push_back(normal);
        normals_.push_back(normal);
    }
    // Create bottom cap.
    for (int i = 0; i < subdiv - 2; i++) {
        normals_.push_back(-normal);
        normals_.push_back(-normal);
        normals_.push_back(-normal);
    }

    // Create barrel
    for (int i = 0; i < subdiv; i++) {
        //Right-side up triangle
        normals_.push_back(glm::normalize(cap_top[i] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i + 1] - bottom_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i] - bottom_centerpoint));
        //Upside-down triangle
        normals_.push_back(glm::normalize(cap_top[i] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_top[i + 1] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i + 1] - bottom_centerpoint));
    }

    // indices and colors
    glm::vec3 color (0.6f, 0.6f, 0.6f);
    for (unsigned int i = 0; i < vertices_.size(); ++i) {
        colors_.push_back(color);
    }

    for (unsigned int i = 0; i < vertices_.size(); ++i) {
        indices_.push_back(i);
    }
}
void Cylinder::add_indice(int v[])
{

}
void Cylinder::add_normal(float v[])
{

}
void Cylinder::add_vertex(float v[])
{

}
void Cylinder::add_texture(float v[])
{

}

float Cylinder::test_intersect_cap(const Ray &ray) const
{
	if(ray.dir.y == 0)
		return -1;

	float x, z;
	float cap_top_y = height_/2.0;
	float cap_bottom_y = -cap_top_y;

	//test cap top
	float t0 = (cap_top_y - ray.orig.y) / ray.dir.y;
	//printf("---------t0 %f\n", t0);
	if(t0 >= 0)
	{
		x = ray.orig.x + ray.dir.x * t0;
		z = ray.orig.z + ray.dir.z * t0;
		//not in cap plane
		if(x*x + z*z > (radius_ * radius_))
			t0 = -1;
	}
	
	//test cap bottom 
	float t1 = (cap_bottom_y - ray.orig.y) / ray.dir.y;
	if(t1 >= 0)
	{
		x = ray.orig.x + ray.dir.x * t1;
		z = ray.orig.z + ray.dir.z * t1;
		//in cap plane
		if(x*x + z*z <= (radius_ * radius_))
		{
			if(t0 >= 0) 
				return std::min(t0, t1);
			return t1;
		}
	}
	return t0;
}

Intersection Cylinder::intersectImpl(const Ray &ray) const
{	
	float cap_top_y = height_/2.0;
	float cap_bottom_y = -cap_top_y;

	//a = xd^2 + zd^2
	float a = (ray.dir.x * ray.dir.x) + (ray.dir.z * ray.dir.z);
	//b = 2xoxd + 2zozd 
	float b = 2*(ray.orig.x * ray.dir.x) + 2*(ray.orig.z * ray.dir.z);
	//c = xe^2 + ze^2 - r^2
	float c = (ray.orig.x * ray.orig.x) + (ray.orig.z * ray.orig.z) - (radius_ * radius_);

	//test intersection with cylinder wall
	vector<float>ts;
	float t0, t1;
	t0 = t1 = -1;
	if(a != 0)
	{
		t0 = (-b + sqrt(b*b - 4*a*c)) / (2*a); 
		if(t0 >= 0)
		{
			float y = ray.orig.y + ray.dir.y * t0;
			if(y >= cap_bottom_y && y <= cap_top_y)
				ts.push_back(t0);
		}
		float t1 = (-b - sqrt(b*b - 4*a*c)) / (2*a); 
		if(t1 >= 0)
		{
			float y = ray.orig.y + ray.dir.y * t1;
			if(y >= cap_bottom_y && y <= cap_top_y)
				ts.push_back(t1);
		}
	}
	
	glm::vec3 p0, p1;
	//test intersection of twos caps
	float t2 = test_intersect_cap(ray);
	if(t2 >= 0)
		ts.push_back(t2);
	
	float t = -1;
	if(ts.size() > 0)
	{
		t = ts[0];	
		for(unsigned int i = 1; i < ts.size(); ++i)
		{
			t =  std::min(t, ts[i]);
		}
	}
	Intersection res;
	res.t = t;

	//compute normal
	if(t >= 0)
	{
		glm::vec3 pos = ray.orig + ray.dir * t;
		if(fabs(pos.y - cap_top_y) < EPS)
		{
			res.normal = glm::vec3(0,1,0);
		}
		else if( fabs(pos.y - cap_bottom_y) < EPS)
		{
			res.normal = glm::vec3(0,1,0);
		}
		else
		{
			res.normal = -glm::vec3(pos.x, 0, pos.z);
		}
	}
	return res; 
}

