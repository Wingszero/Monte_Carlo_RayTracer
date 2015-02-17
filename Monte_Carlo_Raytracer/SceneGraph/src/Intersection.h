#ifndef INTERSECTION_H
#define INTERSECTION_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class Node;

struct Intersection 
{
	public:
		Intersection();
		virtual ~Intersection(); 

		float t;
		Node* node_pt; //mark which object to hit
		glm::vec3 pos; //intersect point position
		glm::vec3 normal;
};

#endif
