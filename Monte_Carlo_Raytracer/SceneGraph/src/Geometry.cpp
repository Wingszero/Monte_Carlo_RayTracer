#include "Geometry.h"

Geometry::Geometry(geometryType geomType) :
    type_(geomType)
{
}

Geometry::~Geometry()
{
    vertices_.clear();
    normals_.clear();
    colors_.clear();
    indices_.clear();
    texture_.clear();
}

void Geometry::init_normals() 
{
	if(normals_.size() > 0) return;
	glm::vec3* norm_array = new glm::vec3[vertices_.size()];
	for (unsigned int i = 0; i < indices_.size(); i+=3) //each face
	{
		//triangle three points
		int ia = indices_[i];
		int ib = indices_[i+1];
		int ic = indices_[i+2];
		glm::vec3 normal = glm::normalize(glm::cross(
		  glm::vec3(vertices_[ib]) - glm::vec3(vertices_[ia]),
		  glm::vec3(vertices_[ic]) - glm::vec3(vertices_[ia])));		
		//norm_array[ia] = norm_array[ib] = norm_array[ic] = normal;
		norm_array[ia] += normal;

		normal = glm::normalize(glm::cross(
		  glm::vec3(vertices_[ic]) - glm::vec3(vertices_[ib]),
		  glm::vec3(vertices_[ia]) - glm::vec3(vertices_[ib])));		
		norm_array[ib] += normal;

		normal = glm::normalize(glm::cross(
		  glm::vec3(vertices_[ia]) - glm::vec3(vertices_[ic]),
		  glm::vec3(vertices_[ib]) - glm::vec3(vertices_[ic])));		
		norm_array[ic] += normal;
	 }

	for (unsigned int i = 0; i < vertices_.size(); ++i) 
	{
		normals_.push_back(glm::normalize(norm_array[i])); 
	}

//	printf("vertex size: %d\n", vertices_.size());
//	printf("normals size: %d\n", normals_.size());
//	printf("indices size: %d\n", indices_.size());
	delete[] norm_array;
}

//init each triangle plane's normal for backface_cull and intersection test
void Geometry::init_tri_normals()
{
	for (unsigned int i = 0; i < indices_.size(); i+=3) //each triangle plane 
	{
		//cross(p1-p0, p2-p0)
		tri_normals.push_back(glm::normalize(glm::cross(vertices_[indices_[i+1]] -  vertices_[indices_[i]],  
			vertices_[indices_[i+2]] -  vertices_[indices_[i]]))); 
	}
}

void Geometry::display()
{
	printf("indices size: %d\n", indices_.size());
	printf("vertex size: %d\n", vertices_.size());
	printf("normals size: %d\n", normals_.size());
	printf("\n");
}

void Geometry::init_bv()
{
	if(type_ == MESH)
	{
		bv = new BV(this->vertices_); 
	}
}

void Geometry::build_kdtree(int max_depth) 
{
	if(type_ == MESH)
	{
		kd_tree = new KdTree(max_depth);	
		kd_tree->build_tree(vertices_, indices_);
	}
}


Intersection Geometry::intersect(const glm::mat4 &T, Ray ray_world) const
{
   // Transform the ray into OBJECT-LOCAL-space, for intersection calculation.
	ray_world.dir = glm::normalize(ray_world.dir);
	glm::vec4 new_orig = glm::inverse(T) * glm::vec4(ray_world.orig, 1);
	glm::vec4 new_dir = glm::inverse(T) * glm::vec4(ray_world.dir, 0);

    Ray ray_local;  
	ray_local.orig = glm::vec3(new_orig.x, new_orig.y, new_orig.z);
	ray_local.dir= glm::vec3(new_dir.x, new_dir.y, new_dir.z);	
	ray_local.dir = glm::normalize(ray_local.dir);

	Intersection isx = intersectImpl(ray_local);
    if (isx.t >= 0.f) 
    //if (isx.t != -1) 
	{
		//transform to world space
		glm::vec4 tmp = T * glm::vec4(isx.normal, 0); 

		glm::vec3 normal_world = glm::vec3(tmp.x, tmp.y, tmp.z);
		//must toward to ray orig
        if(glm::sign(glm::dot(normal_world, ray_world.dir)) > 0)
		{
			normal_world = -normal_world;
		}
		glm::vec4 tmp_pos  = T * glm::vec4(isx.pos, 1);
		isx.pos = glm::vec3(tmp_pos.x, tmp_pos.y, tmp_pos.z);
		isx.normal = glm::normalize(normal_world);
    }
    return isx;
}

