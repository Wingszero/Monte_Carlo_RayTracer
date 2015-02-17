#ifndef CAMERA_H
#define CAMERA_H 

#include "glm/glm.hpp"

class Camera 
{
	public: 
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 up_vec;
		float fov;
		int w,h; //window_wide, height

		void set_position(float x, float y, float z);
		void set_direction(float x, float y, float z);
		void set_up_vec(float x, float y, float z);
		void set_fov(float degree); 
};

#endif
