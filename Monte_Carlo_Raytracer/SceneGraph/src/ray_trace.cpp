#include "ray_trace.h"
#include "omp.h"
#include <cstdio>
#include <string>


RayTrace::RayTrace()
{
	MAX_RECUR = 5; //ray tracing recursive max level 
	AA_SAMPLE = 1; //anti-aliasing sampling rate
	SHADOW_RAY_NUM = 0; //soft shadow ray nums
	MC_MAX = 0; //Monte-Carlo iteration times, 0 means not do Monte-Carlo
	DIRECT_ILLU_WEIGHTS = 0.f; //direct light weights in Monte-Carlo raytrace
}

RayTrace::~RayTrace() 
{
	lights.clear();
}

void RayTrace::input(FILE* pFile) 
{
	char flag[20];
	fscanf(pFile, "%s %d", flag, &MAX_RECUR); 
	fscanf(pFile, "%s %d", flag, &AA_SAMPLE); 
	fscanf(pFile, "%s %d", flag, &SHADOW_RAY_NUM); 
	fscanf(pFile, "%s %d", flag, &MC_MAX); 
	fscanf(pFile, "%s %f", flag, &DIRECT_ILLU_WEIGHTS); 
}

void RayTrace::add_light(Node* light) 
{
	lights.push_back(light);
}

// Returns a random point on a sphere
glm::vec3 getRandomPointOnSphere(Geometry* sphere, glm::mat4 T) 
{
	// generate u, v, in the range (0, 1)
	float u = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	float theta = 2.0f * M_PI * u;
	float phi = acos(2.0f * v - 1.0f);

	// find x, y, z coordinates assuming unit sphere in object space
	glm::vec3 point;
	point[0] = sin(phi) * cos(theta);
	point[1] = sin(phi) * sin(theta);
	point[2] = cos(phi);

	glm::vec4 tmp = T * glm::vec4(point, 1);
	return glm::vec3(tmp[0], tmp[1], tmp[2]);
}

// Returns a random point on a cube. 
glm::vec3 getRandomPointOnCube(Geometry* cube, glm::mat4 T, float scale[]) 
{
	glm::vec3 dim (0.5 * scale[0], 0.5 * scale[1], 0.5 * scale[2]);
	
	// Get surface area of the cube
	float side1 = dim[0] * dim[1];		// x-y
	float side2 = dim[1] * dim[2];		// y-z
	float side3 = dim[0] * dim[2];		// x-z
	float totalArea = 2.0f * (side1 + side2 + side3);	

	// pick random face weighted by surface area
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	// pick 2 random components for the point in the range (-0.5, 0.5)
	float c1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
	float c2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;

	glm::vec3 point;
	if (r < side1 / totalArea) {				
		// x-y front
		point = glm::vec3(c1, c2, 0.5f);
	} else if (r < (side1 * 2) / totalArea) {
		// x-y back
		point = glm::vec3(c1, c2, -0.5f);
	} else if (r < (side1 * 2 + side2) / totalArea) {
		// y-z front
		point = glm::vec3(0.5f, c1, c2);
	} else if (r < (side1 * 2 + side2 * 2) / totalArea) {
		// y-z back
		point = glm::vec3(-0.5f, c1, c2);
	} else if (r < (side1 * 2 + side2 * 2 + side3) / totalArea) {
		// x-z front 
		point = glm::vec3(c1, 0.5f, c2);
	} else {
		// x-z back
		point = glm::vec3(c1, -0.5f, c2);
	}
	glm::vec4 tmp = T * glm::vec4(point, 1);
	return glm::vec3(tmp[0], tmp[1], tmp[2]);
}

// Given a normal vector, find a cosine weighted random direction in a hemisphere
glm::vec3 getCosineWeightedDirection(const glm::vec3& normal) {

	// Pick 2 random numbers in the range (0, 1)
	float xi1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float xi2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    float up = sqrt(xi1); 			// cos(theta)
    float over = sqrt(1 - up * up); // sin(theta)
    float around = xi2 * 2.0f * M_PI;
    
    // Find a direction that is not the normal based off of whether or not the normal's components 
    // are all equal to sqrt(1/3) or whether or not at least one component is less than sqrt(1/3).
    const float SQRT_OF_ONE_THIRD = sqrt(1.0f/3.0f);
    glm::vec3 directionNotNormal;
    if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
      directionNotNormal = glm::vec3(1.f, 0.f, 0.f);
    } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
      directionNotNormal = glm::vec3(0.f, 1.f, 0.f);
    } else {
      directionNotNormal = glm::vec3(0.f, 0.f, 1.f);
    }
    
    //Use not-normal direction to generate two perpendicular directions
    glm::vec3 perpendicularDirection1 = glm::normalize(glm::cross(normal, directionNotNormal));
    glm::vec3 perpendicularDirection2 = glm::normalize(glm::cross(normal, perpendicularDirection1));
    
    return (up * normal) + (cos(around) * over * perpendicularDirection1) + (sin(around) * over * perpendicularDirection2);
}

glm::vec3 get_light_point(Node* light)
{
	if(light->self_geometry->is_cube())
	{
		return getRandomPointOnCube(light->self_geometry, light->self_transmat, light->self_scale);
	}
	if(light->self_geometry->is_sphere())
	{
		return getRandomPointOnSphere(light->self_geometry, light->self_transmat);
	}	
	return glm::vec3(0.f,0.f,0.f);
}

Ray RayTrace::calc_refract_ray(const Ray &in_ray, const Intersection &hit_pt, Node* last_node, bool &has_refr_ray) 
{
	glm::vec3 N = glm::normalize(hit_pt.normal); 
	glm::vec3 I = glm::normalize(in_ray.dir); 
	glm::vec3 T; //out ray dir
	float i12; //Index of refraction

	float refr_index = hit_pt.node_pt->ma->refraction; 
	if(hit_pt.node_pt == last_node) //from obj to air
	{
		i12 = refr_index;
	}
	else //from air to obj
	{
		i12 = 1.0f/refr_index; 
	}
	Ray out_ray;
	float NdotI = glm::dot(N, I); 
	float tmp =  1.0f - i12 * i12 * (1.0f - NdotI * NdotI);
	
	if(tmp >= 0.f)
	{
		tmp = sqrt(tmp);
		T = (-i12 * NdotI - tmp) * N + i12 * I; 
		out_ray.dir = glm::normalize(T);
		out_ray.orig = hit_pt.pos + float(INTER_EPS) * out_ray.dir; //let hit_pt leave the surface of object 
		//has_refr_ray = true;
	}
	else
	{
		//has_refr_ray = true; 
		return calc_reflect_ray(in_ray, hit_pt);
	}

	return out_ray;
}

Ray RayTrace::calc_reflect_ray(const Ray &in_ray, const Intersection &hit_pt)
{
	Ray out_ray;
	glm::vec3 Ri = glm::normalize(in_ray.dir);
	glm::vec3 N = glm::normalize(hit_pt.normal);
	out_ray.dir = glm::normalize(Ri - 2.0f * N * (glm::dot(Ri, N)));  
	out_ray.orig = hit_pt.pos + float(INTER_EPS) * out_ray.dir; 
	return out_ray;
}

bool RayTrace::shadow_ray_unblock(const Intersection &hit_pt, const glm::vec3 &light_pos) 
{
	Ray ray(hit_pt.pos, glm::normalize(light_pos - hit_pt.pos));
	float length  = glm::length(light_pos - hit_pt.pos);
	for (vector<Node*>::iterator iter = each_nodes.begin(); iter != each_nodes.end(); ++iter)
	{
		if((*iter)->self_geometry) 
		{
			if((*iter) == hit_pt.node_pt && hit_pt.node_pt->ma->can_reflect()) 
				continue; //mirror can not block by itself
			if((*iter)->is_light())
				continue; //light is not obstacle
			Intersection tmp = (*iter)->self_geometry->intersect((*iter)->self_transmat, ray); 
			if (tmp.t >= SHADOW_EPS && glm::length(tmp.pos - hit_pt.pos) < length)  //between obj0 and light
			{	
				return false;
			}
		}
	}
	return true;
}

glm::vec3 RayTrace::calc_color(const Intersection &hit_pt, glm::vec3 lpos, glm::vec3 spec, glm::vec3 KdColor) 
{
	float expo = hit_pt.node_pt->ma->expo;

	//diffuse
	glm::vec3 L = glm::normalize(lpos - hit_pt.pos);
	float NdotL = glm::dot(hit_pt.normal, L);
	NdotL = std::max(0.f, NdotL);
	glm::vec3 diff_term = KdColor * NdotL; 

	//specular	
	if(!hit_pt.node_pt->ma->can_reflect()) //not mirror
	{
		glm::vec3 spec_col = hit_pt.node_pt->ma->specular;
		if(spec_col.x > 0.f || spec_col.y > 0.f || spec_col.z > 0.f)
		{
			Ray in_ray;
			in_ray.dir = glm::normalize(hit_pt.pos - lpos); //from light to hit point
			Ray ref_ray = calc_reflect_ray(in_ray, hit_pt);
			float CosTheta =  glm::dot(ref_ray.dir, -camera->direction);
			if(CosTheta > 0.f)
				spec =  spec_col * pow(CosTheta, expo);
		}
	}
	return diff_term + spec * SPEC_FAC;	
}

void RayTrace::ray_trace(const Ray &ray, int level, glm::vec3 &color, Node* last_node) 
{
	color = glm::vec3(0.f, 0.f, 0.f); 
	
	if(level > MAX_RECUR) return;

	//get nearesr intersection point
	Intersection hit_pt = get_intersect_point(ray, last_node);			
	glm::vec3 spec(0,0,0); 
	glm::vec3 refr(0,0,0); //color component
	
	//hit object
	if(hit_pt.node_pt)
	{
		//is_light
		if(hit_pt.node_pt->is_light())
		{
			color = hit_pt.node_pt->ma->diffuse;
			return;
		}

		//reflect 
		if(hit_pt.node_pt->ma->can_reflect())	
		{
			Ray next_ray =  calc_reflect_ray(ray, hit_pt); 
			glm::vec3 new_color; 

			ray_trace(next_ray, level + 1, new_color, hit_pt.node_pt);
				
			spec = new_color * hit_pt.node_pt->ma->specular;
		}	

		//refract
		if(hit_pt.node_pt->ma->can_refract())	
		{
			bool has_ray;
			Ray next_ray = calc_refract_ray(ray, hit_pt, last_node, has_ray); 
			glm::vec3 new_color;

			ray_trace(next_ray, level + 1, new_color, hit_pt.node_pt);

			refr = REFR_FAC * new_color;
		}
	
		//both mirr and transparent
		if(hit_pt.node_pt->ma->can_reflect() && hit_pt.node_pt->ma->can_refract())	
		{
			spec *= 0.3f;
			refr *= 0.7f;
		}

		//shadow 
		glm::vec3 amb = AMB_FAC * hit_pt.node_pt->ma->diffuse; //ambient
		color =  amb + refr;
		
		
		unsigned int light_num = lights.size(); 
		for(unsigned int i = 0; i < light_num; ++i)
		{
			Node* light = lights[i];
			glm::vec3 lcol = light->ma->diffuse; 
			glm::vec3 lpos;
			glm::vec3 shadow_col(0,0,0);
			if(SHADOW_RAY_NUM > 0) //area light
			{
				for(int j = 0; j < SHADOW_RAY_NUM; ++j)
				{
					lpos = get_light_point(light); 
					if (shadow_ray_unblock(hit_pt, lpos))
					{
						glm::vec3 KdColor = DIFF_FAC * hit_pt.node_pt->ma->diffuse;
						shadow_col += calc_color(hit_pt, lpos, spec, KdColor);		
					}
					//else add black color---->(0,0,0)
				}
				color += (shadow_col * lcol / (float(light_num * SHADOW_RAY_NUM)));
			}
			else //point light
			{
				lpos.x = light->real_center.x; 
				lpos.y = light->real_center.y; 
				lpos.z = light->real_center.z; 
				if (shadow_ray_unblock(hit_pt, lpos))
				{
					glm::vec3 KdColor = DIFF_FAC * hit_pt.node_pt->ma->diffuse;
					shadow_col += calc_color(hit_pt, lpos, spec, KdColor);		
				}
				//else add black color---->(0,0,0)
				color += (shadow_col * lcol / float(light_num));
			}
		}
	}
	else //no hit obj
	{
		glm::vec3 dir = glm::normalize(ray.dir);
		int num = 0;
		glm::vec3 tmp(0,0,0);
		for(unsigned int i = 0; i < lights.size(); ++i)
		{
			//intersect light source
			Node* light = lights[i];
			glm::vec3 lpos = glm::vec3(light->real_center[0], light->real_center[1], light->real_center[2]);
			if(dir == glm::normalize(lpos - ray.orig)) 
			{
				tmp += light->ma->diffuse;
				num++;
			}
		}
		if(num > 0)
			color += (tmp/(float)num);
	}
}

//Monte-Carlo ray tracing
void RayTrace::mc_ray_trace(const Ray &ray, int level, glm::vec3 &color, Node* last_node, bool direct_illu) 
{
	color = glm::vec3(0.f, 0.f, 0.f); 
	if(level > MAX_RECUR) return;
	/*
	if(direct_illu && ray.hit_diff_cnt > 1) //direct illumination
	{
		//printf("j");
		return;
	}
	*/

	Intersection hit_pt = get_intersect_point(ray, last_node);			
	//no intersection
	if(!hit_pt.node_pt) return;
	
	//color component
	glm::vec3 diff_col(0.f,0.f,0.f); 
	glm::vec3 spec(0.f,0.f,0.f); 
	glm::vec3 refr(0.f,0.f,0.f); 

	//hit light
	if(hit_pt.node_pt->is_light())
	{
		/*
		if(!direct_illu && ray.hit_diff_cnt < 2) //indirect illumination
		{
			return;
		}
		*/
		Material * light_ma = hit_pt.node_pt->ma;
		color = ray.trans * light_ma->diffuse * light_ma->emit;
		return;
	}

	//reflect material
	if(hit_pt.node_pt->ma->can_reflect())	
	{
		Ray next_ray =  calc_reflect_ray(ray, hit_pt); 
		next_ray.trans = ray.trans * hit_pt.node_pt->ma->specular;
		glm::vec3 new_color; 

		mc_ray_trace(next_ray, level + 1, new_color, hit_pt.node_pt, direct_illu);

		spec = SPEC_FAC * new_color * hit_pt.node_pt->ma->specular;
	}	

	//refract material
	if(hit_pt.node_pt->ma->can_refract())	
	{
		bool has_refract;
		Ray next_ray = calc_refract_ray(ray, hit_pt, last_node, has_refract); 
		next_ray.trans = ray.trans; 
		glm::vec3 new_color;

		mc_ray_trace(next_ray, level + 1, new_color, hit_pt.node_pt, direct_illu);

		refr = REFR_FAC * new_color;
	}

	//both mirr and transparent
	if(hit_pt.node_pt->ma->can_reflect() && hit_pt.node_pt->ma->can_refract())	
	{
		spec *= 0.3f;
		refr *= 0.7f;
	}

	//Lambert material 
	if(hit_pt.node_pt->ma->is_diffuse())
	{
		float p = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		if(p <= hit_pt.node_pt->ma->absorb) // absorbed
		{
			return;
		}
		else
		{
			//reflect ray direction
			glm::vec3 dir = glm::normalize(getCosineWeightedDirection(hit_pt.normal));
			Ray next_ray(hit_pt.pos, dir);
			next_ray.trans = ray.trans * hit_pt.node_pt->ma->diffuse / (1.f - hit_pt.node_pt->ma->absorb);
			next_ray.hit_diff_cnt = ray.hit_diff_cnt + 1;
			glm::vec3 new_color;

			mc_ray_trace(next_ray, level + 1, new_color, hit_pt.node_pt, direct_illu);

			//use the new color as the light source of diffuse surface
			diff_col = DIFF_FAC * new_color * hit_pt.node_pt->ma->diffuse;
		}
	}

	glm::vec3 amb = AMB_FAC * hit_pt.node_pt->ma->diffuse; //ambient
	color +=  amb + refr + diff_col;
	
	//specular color
	unsigned int light_num = lights.size(); 
	glm::vec3 spec_col(0.f,0.f,0.f);
	for(unsigned int i = 0; i < light_num; ++i)
	{
		//multiply light_color
		spec_col += (spec * lights[i]->ma->diffuse);	
	}
	color += (spec_col/ float(light_num));
}


Intersection RayTrace::get_intersect_point(const Ray& ray, Node* last_node) 
{
	Intersection res;
	res.t = FLT_MAX; 
	for (vector<Node*>::iterator iter = each_nodes.begin(); iter != each_nodes.end(); ++iter)
	{
		if((*iter)->self_geometry) 
		{
			Intersection tmp;
			tmp = (*iter)->self_geometry->intersect((*iter)->self_transmat, ray); 
			if (tmp.t >= INTER_EPS && tmp.t < res.t)  //find the nearest intersect
			{
				res.t =tmp.t;
				res.normal = tmp.normal;
				res.pos = tmp.pos;
				res.node_pt = (*iter);
			}
		}
	}
	if(res.t == FLT_MAX)
	{
		res.t = -1;
	}
	return res;
}

void RayTrace::enter(Camera camera_ins,   vector<Node*>&node_list) 
{
	clock_t newtime, oldtime;
	oldtime = clock();

	camera = &camera_ins;
	each_nodes = node_list;
	//float width, height;
	const int width = camera->w;
	const int height = camera->h;

	//view ray calculation
	float fovy = camera->fov; 
	glm::vec3 A = glm::cross(camera->direction, camera->up_vec);
	glm::vec3 B = glm::cross(A, camera->direction);
	glm::vec3 M = camera->position + camera->direction;
	float C_len = glm::length(camera->direction);
	float ratio1 = C_len * tan(fovy * PI/ 180.0f);  
	float ratio2 = C_len * tan(fovy * PI/ 180.0f) * float(width)/float(height);	
	glm::vec3 V = (B * ratio1) / glm::length(B); //vertical vec
	glm::vec3 H = (A* ratio2) / glm::length(A); //horizontal vec
	
	//color cache. For Anti-aliasing
	float ***res_col = new float**[width];
	for(int i = 0; i < width; ++i)
	{
		res_col[i] = new float*[height];
		for(int j = 0; j < height; ++j)
		{
			res_col[i][j] = new float[RGBN];
			for(int k = 0; k < RGBN; ++k)
				res_col[i][j][k] = 0.f;
		}
	}
	int plan = 0;
	#pragma omp parallel for
	for(int i = 0; i < width; ++i)
	{
		for(int j= 0; j < height; ++j)
		{
			int k ;
			for(k = 0; k < AA_SAMPLE; ++k) //anti-aliasing
			{
				float i_pos, j_pos;
				if(k == 0) //original pixel
				{
					i_pos = i;
					j_pos = j; 
				}
				else //random sampling
				{
					// [i - 0.25, i + 0.25]
					i_pos = i - 0.25f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					j_pos = j - 0.25f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				}
			
				glm::vec3 fac_w = H *  ((i_pos  * 2.0f)/float(width) - 1.0f); 
				//calc ray dir
				glm::vec3 P = M +  fac_w + ((j_pos * 2.0f)/(height) - 1.0f) * (-V);

				Ray ray(camera->position,  glm::normalize(P - camera->position));
				glm::vec3 color(0.f, 0.f, 0.f);
				
				//Monte-Carlo ray tracing
				if(MC_MAX >= 1)
				{
					//one-time direct illumination
					glm::vec3 tmp_col;
					if(DIRECT_ILLU_WEIGHTS > 0.f)
					{
						ray_trace(ray, 0, tmp_col, NULL);
						color = tmp_col * DIRECT_ILLU_WEIGHTS; 
					}
					
					//many-times indirect illumination
					tmp_col = glm::vec3(0.f, 0.f, 0.f);
					for(int mc = 0; mc < MC_MAX; ++mc) //Monte-Carlo
					{
						glm::vec3 col;
						mc_ray_trace(ray, 0, col, NULL, false);
						tmp_col += col;
					}
					tmp_col /= (float)MC_MAX;
					color += (tmp_col * (1.f - DIRECT_ILLU_WEIGHTS));
					//color += (tmp_col);
				}
				else
				{
					ray_trace(ray, 0, color, NULL);
				}

				float tmp[3];
				tmp[0] = (std::min(1.0f, std::max(0.f, color.x)));
				tmp[1] = (std::min(1.0f, std::max(0.f, color.y)));
				tmp[2] = (std::min(1.0f, std::max(0.f, color.z)));

				//AA: adative sampling, need improvement 
				/*
				if(k >= 1)
				{
					bool break_sampling = true;
					for(int r = 0; r < RGBN; ++r)
					{
						//color difference is not acceptable
						if(abs(res_col[i][j][r]/(float)(k) - tmp[r]) >= 0.001f)
						{
							break_sampling = false;
							break;
						}
					}
					if(break_sampling) break;
				}
				*/
				for(int r = 0; r < RGBN; ++r)
				{
					res_col[i][j][r] += tmp[r]; 
				}	
			} //AA ends

			for(int r = 0; r < RGBN; ++r)
				res_col[i][j][r] /= (float)k; 
		}
		++plan;
		if(plan % 20 == 0)
		{
			printf("Thread:%d rendering... %d %% \n", omp_get_thread_num(), 
				int(float(plan * omp_get_num_threads()) / float(width) * 100));
		}
	}	
	newtime = clock();

	//printf("%d\n", plan);
	//init output
	BMP output;
	output.SetSize(width, height);
	output.SetBitDepth(24);
	
	for(int i = 0; i < width; ++i)
	{
		for(int j = 0; j < height; ++j)
		{
			output(i, j)->Red = res_col[i][j][0] * RGBV;
			output(i, j)->Green= res_col[i][j][1] * RGBV; 
			output(i, j)->Blue= res_col[i][j][2] * RGBV; 
		}
	}
	
	float usetime = static_cast<float>(newtime - oldtime) / static_cast<float>(CLOCKS_PER_SEC);
	std::string s = 
		"Id" + std::to_string(long double(newtime)) +
		"_AA" + std::to_string(long double(AA_SAMPLE)) +  //anti-aliasing
		"_SHD" + std::to_string(long double(SHADOW_RAY_NUM)) + //shadow ray num 
		"_MC" + std::to_string(long double(MC_MAX)) + //Monte-Carlo times 
		"_DI" + std::to_string(long double(DIRECT_ILLU_WEIGHTS * 100)) + //Direct illumination Weights
		"_SEC" + std::to_string(long double(usetime)) + ".bmp"; //time consumed

	//std::string s = "test.bmp";
	char const *file_name = s.c_str(); 
	output.WriteToFile(file_name);

	printf("Total used time: %.2f seconds.", usetime);
	printf("\n -----------------RayTracing End! --------------------------------\n");
	
	//clear data
	for(int i = 0; i < width; ++i)
	{
		//res_col[i] = new float*[height];
		for(int j = 0; j < height; ++j)
		{
			delete[] res_col[i][j];
		}
		delete[] res_col[i];
	}
	delete[] res_col;

	//debug
	//exit(0);
}