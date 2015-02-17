#include "camera.h"

void Camera::set_direction(float x, float y, float z)
{
	direction.x = x;	
	direction.y = y; 
	direction.z = z; 
	direction = glm::normalize(direction);
}

void Camera::set_position(float x, float y, float z)
{
	position.x = x;	
	position.y = y; 
	position.z = z; 
}

void Camera::set_up_vec(float x, float y, float z)
{
	up_vec.x = x;	
	up_vec.y = y; 
	up_vec.z = z; 
}
void Camera::set_fov(float degree) 
{
	fov = degree;	
}
