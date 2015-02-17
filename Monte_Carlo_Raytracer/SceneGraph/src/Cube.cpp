#include "Cube.h"


const float VER_COORD[8][3] = 
{
	{-1.0, -1.0,  1.0}, 
	{1.0, -1.0,  1.0}, 
	{1.0, 1.0,  1.0}, //max_pt
	{-1.0, 1.0,  1.0}, 
	{-1.0, -1.0,  -1.0},  //min_pt
	{1.0, -1.0,  -1.0},
	{1.0, 1.0,  -1.0}, 
	{-1.0, 1.0,  -1.0}, 
};

const float VER_COLORS[8][3] = 
{
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 0.0},
};

const unsigned int VER_INDICE[12][3] = 
{
    // front
	{0, 1, 2},
    {2, 3, 0},
    // top
    {1, 5, 6},
    {6, 2, 1},
    // back
    {7, 6, 5},
    {5, 4, 7},
    // bottom
    {4, 0, 3},
    {3, 7, 4},
    // left
    {4, 5, 1},
    {1, 0, 4},
    // right
    {3, 2, 6},
    {6, 7, 3},
};

const unsigned int VER_NORMALS[12][3] = 
{
    // front
	{0, 0, 1},
	{0, 0, 1},
    // top
    {0, 1, 0},
    {0, 1, 0},
    // back
	{0, 0, -1},
	{0, 0, -1},
    // bottom
    {0, -1, 0},
    {0, -1, 0},
    // left
    {-1, 0, 0},
    {-1, 0, 0},
    // right
    {1, 0, 0},
    {1, 0, 0},
};


Cube::Cube() :
    Geometry(CUBE),
    center_(glm::vec3(0.f, 0.f, 0.f)),
    length_(1.f)
{
    buildGeomtery();
}

Cube::~Cube() {}

void Cube::buildGeomtery()
{
    vertices_.clear();
    colors_.clear();
    normals_.clear();
    indices_.clear();	
	float half_length = length_/ 2.0f;
	for(int i = 0; i < 8; ++i)
	{
		//coord
		float  x,y,z;
		x = (VER_COORD[i][0] * half_length) - center_.x;
		y = (VER_COORD[i][1] * half_length) - center_.y;
		z = (VER_COORD[i][2] * half_length) - center_.z;
		glm::vec3 coord(x,y,z);
		vertices_.push_back(coord);
		//colors
		glm::vec3 colors(VER_COLORS[i][0], VER_COLORS[i][1], VER_COLORS[i][2]);
		colors_.push_back(colors);
	}
	float min_coord = -half_length; 
	float max_coord = half_length; 
	min_pt.x = min_pt.y = min_pt.z = min_coord;
	max_pt.x = max_pt.y = max_pt.z = max_coord;

	//triangles 
	for(int i = 0; i < 12; ++i)
	{
		indices_.push_back(VER_INDICE[i][0]);
		indices_.push_back(VER_INDICE[i][1]);
		indices_.push_back(VER_INDICE[i][2]);
		glm::vec3 normal(VER_NORMALS[i][0], VER_NORMALS[i][1], VER_NORMALS[i][2]);
		normals_.push_back(normal);
	}
}
 
void Cube::add_indice(int v[])
{

}
void Cube::add_normal(float v[])
{

}
void Cube::add_vertex(float v[])
{

}
void Cube::add_texture(float v[])
{

}

//debug
//int tt = 0;
Intersection Cube::intersectImpl(const Ray &ray) const
{
	float t_near = -FLT_MAX; 
	float t_far = FLT_MAX; 
	float real_near = FLT_MAX;
	glm::vec3 tmp_1, tmp_2;	

	Intersection false_res;
	false_res.t = -1;

	// parallel to yz 
	if (fabs(ray.dir.x) < 0)
	{
            if ((ray.orig.x < min_pt.x +EPS) || (ray.orig.x > max_pt.x + EPS)) 
			{
                return false_res; 
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
                return false_res;
            }
	}
	// parallel to xz 
	if (fabs(ray.dir.y) < EPS)
	{
            if ((ray.orig.y < min_pt.y + EPS) || (ray.orig.y > max_pt.y + EPS)) 
			{
                return false_res; 
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
                return false_res;
            }
	}
	// parallel to xy 
	if (fabs(ray.dir.z) < EPS)
	{
            if ((ray.orig.z < min_pt.z + EPS) || (ray.orig.z > max_pt.z + EPS)) 
			{
                return false_res; 
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
                return false_res;
            }
	}
	Intersection res;
	res.t = t_near;
	//printf("t: %f ", t_near);

	//computer normal
	glm::vec3 pos = ray.orig + t_near * ray.dir;
	res.pos = pos;
	float dis = length_ / 2.0f;
	res.normal = glm::vec3(0, 0, 0);

	//compute normal
	if(fabs(fabs(pos.x) - dis) < EPS) //left-right plane
	{
		res.normal = glm::vec3(pos.x, 0, 0);
	}
	else if(fabs(fabs(pos.y) - dis) < EPS) //top-bottom plane
	{
		res.normal = glm::vec3(0, pos.y, 0);
	}
	else if(fabs(fabs(pos.z) - dis) < EPS) //front-behine plane
	{
		res.normal = glm::vec3(0, 0, pos.z);
	}
	else
	{
//		if(tt < 20)
//		{
//			++tt;
//			printf("pos:%f %f %f %f \n", pos.x, pos.y, pos.z, dis);
//		}
	}
	return res; 
}
