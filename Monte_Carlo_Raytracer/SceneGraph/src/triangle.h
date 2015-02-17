#ifndef TRIANGLE_H
#define TRIANGLE_H 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Triangle 
{
    inline Triangle()
	{

	}
    inline Triangle(const glm::vec3 &a_in, const glm::vec3 &b_in, const glm::vec3 &c_in, int id)
	{
		p[0] = a_in;
		p[1] = b_in;
		p[2] = c_in;
		index = id;

		min_pt[0] = std::min(a_in.x, std::min(b_in.x, c_in.x));
		min_pt[1] = std::min(a_in.y, std::min(b_in.y, c_in.y));
		min_pt[2] = std::min(a_in.z, std::min(b_in.z, c_in.z));
	}
	glm::vec3 p[3];
	int index;
	float min_pt[3]; //min point, for Kd-Tree to find median
};

#endif
