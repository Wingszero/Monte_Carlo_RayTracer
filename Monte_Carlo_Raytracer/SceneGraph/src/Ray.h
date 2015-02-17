#ifndef RAY_H
#define RAY_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 orig;
	glm::vec3 dir;
	glm::vec3 trans; //transmittance, use for Monte-Carlo
	int hit_diff_cnt; //hit diffuse material counts

	inline Ray()
	{
		trans = glm::vec3(1.f,1.f,1.f);
		hit_diff_cnt = 0;
	}

	inline Ray(glm::vec3 o, glm::vec3 d)
		: orig(o), dir(d)
	{
		trans = glm::vec3(1.f,1.f,1.f);
		hit_diff_cnt = 0;
	}
};

#endif
