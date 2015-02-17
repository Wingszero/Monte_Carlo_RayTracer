#include "Mesh.h"

Mesh::Mesh() : Geometry(MESH)
{
	bv = NULL;
	kd_tree = NULL;
}

Mesh::~Mesh() 
{
}

void Mesh::buildGeomtery()
{
    vertices_.clear();
    colors_.clear();
    normals_.clear();
    indices_.clear();	
}
 
void Mesh::add_indice(int vi[]) 
{
	for(int i = 0; i < 3; ++i)
	{
		indices_.push_back(vi[i]);
	}
}

void Mesh::add_texture(float v[])  //texture coordinate
{
	glm::vec2 texture_coord(v[0], v[1]);
	texture_.push_back(texture_coord);
}

void Mesh::add_vertex(float v[]) 
{
	glm::vec3 new_vec(v[0], v[1], v[2]);
	vertices_.push_back(new_vec);
}

void Mesh::add_normal(float v[]) 
{
	glm::vec3 new_norm(v[0], v[1], v[2]);
	normals_.push_back(new_norm);
}

//for unit test
Intersection Mesh::triangle_intersect_basic(glm::mat4 const& T, const Ray &ray, glm::vec3 const& p0, 
		glm::vec3 const& p1, glm::vec3 const& p2, bool is_ray_local) const
{
	Intersection false_res;
	false_res.t = -1;
    
	Ray ray_local = ray;  
	if(!is_ray_local)
	{
		glm::vec4 new_orig = glm::inverse(T) * glm::vec4(ray_local.orig, 1);
		glm::vec4 new_dir = glm::inverse(T) * glm::vec4(glm::normalize(ray_local.dir), 0);

		ray_local.orig = glm::vec3(new_orig.x, new_orig.y, new_orig.z);
		ray_local.dir= glm::normalize(glm::vec3(new_dir.x, new_dir.y, new_dir.z));
	}

	// plane normal
	glm::vec3 pn = glm::normalize(glm::cross(p1 - p0, p2 - p0)); 
    float n_dot_ray = glm::dot(ray_local.dir, pn);

	//parallel 
	if(n_dot_ray == 0)
        return false_res; 

	//finding intersection point 
    float d = glm::dot(pn, p0);
	float t = -(-d + glm::dot(pn, ray_local.orig)) / n_dot_ray;
    if (t < 0)
		return false_res; // the triangle is behind 

	//intersection point
    glm::vec3 ix_p = ray_local.orig + t * ray_local.dir;
 
    // inside-outside test
    glm::vec3 C; 

    C = glm::cross(p1 - p0, ix_p - p0);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
    C = glm::cross(p2 - p1,  ix_p - p1);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
    C = glm::cross(p0 - p2, ix_p - p2);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
	Intersection res;
	res.t = t; 
	res.normal = pn;
	return res;
}

inline Intersection Mesh::triangle_intersect(const Ray &ray, int normal_index) const
{
	Intersection false_res;
	false_res.t = -1;
	
	glm::vec3 p0, p1, p2;
	int vid = normal_index * 3; 
	p0 = vertices_[indices_[vid]];
	p1 = vertices_[indices_[vid+1]];
	p2 = vertices_[indices_[vid+2]];
	
	glm::vec3 pn =  tri_normals[normal_index];
    float n_dot_ray = glm::dot(ray.dir, pn);
	//parallel 
	if(n_dot_ray == 0)
        return false_res; 

	//finding intersection point 
    float d = glm::dot(pn, p0);
	
	float t = (d - glm::dot(pn, ray.orig)) / n_dot_ray;
	// triangle is behind the ray 
    if (t < 0)
		return false_res; 

	//intersection point
    glm::vec3 ix_p = ray.orig + t * ray.dir;
 
    // inside-outside test
    glm::vec3 C; 

    C = glm::cross(p1 - p0, ix_p - p0);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
    C = glm::cross(p2 - p1,  ix_p - p1);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
    C = glm::cross(p0 - p2, ix_p - p2);
    if (glm::dot(pn, C) < 0)
        return false_res; 
	
	Intersection res;
	res.t = t; 
	res.normal = pn;
	return res;
}

bool Mesh::check_bv(const Ray &ray) const
{
	return  bv->is_intersect(ray); 
}

//do not use in refraction material
bool Mesh::backface_cull(const glm::vec3 raydir, int normal_index) const 
{
	//if true: drop the backside triangle
	return glm::dot(raydir, tri_normals[normal_index]) >= 0;
}

Intersection Mesh::intersectImpl(const Ray &ray) const
{
	Intersection res,tmp; 
	res.t = FLT_MAX;

	if(kd_tree)
	{
		vector<KdNode*>nodes;
		if(kd_tree->search(ray, nodes)) 
		{	
			for (unsigned int i = 0; i < nodes.size(); ++i) //each kdtree-node 
			{
				for(int j = 0; j < nodes[i]->tris.size(); ++j) //each triangle 
				{
					int tri_id = nodes[i]->tris[j]->index; 
					if(backface_cull(ray.dir, tri_id)) continue;
					tmp = triangle_intersect(ray, tri_id); 

					if(tmp.t >= EPS && tmp.t < res.t)
					{
						res = tmp;
					}	
				}
			}
		}
	}
	else
	{
		//outside the bounding volume
		if(!check_bv(ray))  
		{
			res.t = -1;
			return res;
		}
		for (unsigned int i = 0; i < indices_.size(); i+=3) //each triangle plane 
		{
			int normal_index = i/3;
			if(backface_cull(ray.dir, normal_index)) continue;
			tmp = triangle_intersect(ray, normal_index); 
			if(tmp.t >= EPS && tmp.t < res.t)
			{
				res = tmp;
			}	
		}
	}

	if(res.t == FLT_MAX)
	{
		res.t = -1;
	}
	return res; 
}


