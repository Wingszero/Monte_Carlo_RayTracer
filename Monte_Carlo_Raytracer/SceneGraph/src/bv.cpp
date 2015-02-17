#include "bv.h"

BV::BV()
{
	use_aabb = true;
}

BV::BV(vector<glm::vec3>&all_points)
{
	use_aabb = true;
	if(use_aabb)
	{
		init_box(all_points);
	}
	else
	{
		init_sphere(all_points);
	}
}

BV::BV(vector<Triangle*>&all_tris)
{
	use_aabb = true;
	float min_x, max_x, min_y, max_y, min_z, max_z;
	min_x = min_y = min_z= FLT_MAX;
	max_x = max_y = max_z = -FLT_MAX;
	
	//calc min,max_pt
	for (vector<Triangle*>::iterator iter = all_tris.begin(); iter != all_tris.end(); ++iter) 
	{
		for(int i = 0; i < 3; ++i)
		{
			min_x = std::min(min_x, (*iter)->p[i].x);	
			max_x = std::max(max_x, (*iter)->p[i].x);	

			min_y = std::min(min_y, (*iter)->p[i].y); 
			max_y = std::max(max_y, (*iter)->p[i].y); 

			min_z = std::min(min_z, (*iter)->p[i].z); 
			max_z = std::max(max_z, (*iter)->p[i].z); 
		}
	}
	max_pt = glm::vec3(max_x, max_y, max_z);
	min_pt = glm::vec3(min_x, min_y, min_z);
}


BV::~BV() {}

void BV::init_box(vector<glm::vec3>&all_points)
{
	float min_x, max_x, min_y, max_y, min_z, max_z;
	min_x = min_y = min_z= FLT_MAX;
	max_x = max_y = max_z = -FLT_MAX;
	
	for (vector<glm::vec3>::iterator iter = all_points.begin(); iter != all_points.end(); ++iter) 
	{
		min_x = std::min(min_x, (*iter).x);	
		max_x = std::max(max_x, (*iter).x);	

		min_y = std::min(min_y, (*iter).y);	
		max_y = std::max(max_y, (*iter).y);	

		min_z = std::min(min_z, (*iter).z);	
		max_z = std::max(max_z, (*iter).z);	
	}
	max_pt = glm::vec3(max_x, max_y, max_z);
	min_pt = glm::vec3(min_x, min_y, min_z);
}

void BV::init_sphere(vector<glm::vec3>&all_points)
{
	float min_x, max_x, min_y, max_y, min_z, max_z;
	min_x = min_y = min_z= FLT_MAX;
	radius_ = max_x = max_y = max_z = -FLT_MAX;
	
	for (vector<glm::vec3>::iterator iter = all_points.begin(); iter != all_points.end(); ++iter) 
	{
		min_x = std::min(min_x, (*iter).x);	
		max_x = std::max(max_x, (*iter).x);	

		min_y = std::min(min_y, (*iter).y);	
		max_y = std::max(max_y, (*iter).y);	

		min_z = std::min(min_z, (*iter).z);	
		max_z = std::max(max_z, (*iter).z);	
	}
	center_ = glm::vec3((min_x + max_x)/ 2.0f, (min_y + max_y)/ 2.0f,  (min_z + max_z)/ 2.0f);
	for (vector<glm::vec3>::iterator iter = all_points.begin(); iter != all_points.end(); ++iter) 
	{
		radius_ = std::max(radius_, glm::distance(*iter, center_)); 
	}
	radius_ += EPS;
}

bool BV::is_intersect(const Ray &ray) 
{	
	if(use_aabb)
	{
		//aabb
		return aabb_intersect_test(ray);
	}
	else
	{
		//sphere
		return sphere_intersect_test(ray);
	}
}

bool BV::sphere_intersect_test(const Ray &ray) 
{
	glm::vec3 L = center_ - ray.orig;
	float tca = glm::dot(L, ray.dir);
	if (tca < EPS) 
		return false;

	float d2 = glm::dot(L,L) - tca * tca;
	return d2 <= (radius_ * radius_) + EPS;
}

int BV::get_longest_axis()
{
	glm::vec3 res = max_pt - min_pt; 
	if(res.x >= res.y && res.x >= res.z)
	{
		return 0; //x
	}
	if(res.y >= res.x && res.y >= res.z)
	{
		return 1; //y
	}
	if(res.z >= res.x && res.z >= res.y)
	{
		return 2; //z
	}	
	return 0;
}

inline bool BV::aabb_intersect_test(const Ray &ray) 
{
	float t_near = -FLT_MAX; 
	float t_far = FLT_MAX; 
	float real_near = FLT_MAX;
	glm::vec3 tmp_1, tmp_2;	

	// parallel to yz 
	if (fabs(ray.dir.x) < 0)
	{
		if ((ray.orig.x < min_pt.x +EPS) || (ray.orig.x > max_pt.x + EPS)) 
		{
			return false; 
		}
	} 
	else 
	{ 
		tmp_1.x = (min_pt.x - ray.orig.x) / ray.dir.x;
		tmp_2.x = (max_pt.x - ray.orig.x) / ray.dir.x;
		
		if(tmp_1.x > tmp_2.x + EPS) 
			std::swap(tmp_1,tmp_2);
		t_near = std::max(t_near, tmp_1.x);
		t_far = std::min(t_far, tmp_2.x);
		if( (t_near > t_far + EPS) || (t_far < EPS) )
		{
			return false;
		}
	}
	// parallel to xz 
	if (fabs(ray.dir.y) < EPS)
	{
		if ((ray.orig.y < min_pt.y + EPS) || (ray.orig.y > max_pt.y + EPS)) 
		{
			return false; 
		}
	} 
	else 
	{ 
		tmp_1.y = (min_pt.y - ray.orig.y) / ray.dir.y;
		tmp_2.y = (max_pt.y - ray.orig.y) / ray.dir.y;
		
		if(tmp_1.y > tmp_2.y + EPS) 
			std::swap(tmp_1,tmp_2);
		t_near = std::max(t_near, tmp_1.y);
		t_far = std::min(t_far, tmp_2.y);
		if( (t_near > t_far + EPS) || (t_far < EPS) )
		{
			return false;
		}
	}
	// parallel to xy 
	if (fabs(ray.dir.z) < EPS)
	{
		if ((ray.orig.z < min_pt.z + EPS) || (ray.orig.z > max_pt.z + EPS)) 
		{
			return false; 
		}
	} 
	else 
	{ 
		tmp_1.z = (min_pt.z - ray.orig.z) / ray.dir.z;
		tmp_2.z = (max_pt.z - ray.orig.z) / ray.dir.z;
		
		if(tmp_1.z > tmp_2.z + EPS) 
			std::swap(tmp_1,tmp_2);
		t_near = std::max(t_near, tmp_1.z);
		t_far = std::min(t_far, tmp_2.z);
		if( (t_near > t_far + EPS) || (t_far < EPS) )
		{
			return false;
		}
	}
	return true;
}
