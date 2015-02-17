#ifndef RAYTRACE_H 
#define RAYTRACE_H 

#include <vector>
#include "EasyBMP.h"
#include <cstdio>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Node.h"
#include "Intersection.h"
#include "time.h"

#define RGBV 255 
#define BOX_SIZE 1.0
#define RGBN 3
#define AMB_FAC 0.2f
#define DIFF_FAC 1.f
#define SPEC_FAC 1.f
#define REFR_FAC 1.0f //transmittance, effect the refraction
#define INTER_EPS 1e-3f
#define SHADOW_EPS 1e-3f
#define M_PI 3.14159265 
#define LIGHT_TRANS_EPS 1e-2f

class RayTrace
{
	public:
		RayTrace();
		~RayTrace();
	
		Intersection get_intersect_point(const Ray& ray ,Node* last_node); 
		
		//entry
		void enter(Camera camera_ins,  vector<Node*>&node_list); 
		
		//recursive raytrace
		void ray_trace(const Ray &ray, int level, glm::vec3 &color, Node* last_node); 
		
		//reflection
		Ray calc_reflect_ray(const Ray &in_ray, const Intersection &hit_pt);
		
		//refraction	
		Ray calc_refract_ray(const Ray &in_ray, const Intersection &hit_pt, Node* last_node, bool &has_refr_ray);
		
		//shadow
		bool shadow_ray_unblock(const Intersection &hit_pt, const glm::vec3 &p1); 
		glm::vec3 calc_color(const Intersection &hit_pt, glm::vec3 lpos, glm::vec3 spec, glm::vec3 KdColor); 
	
		//scene
		Camera *camera;
		vector<Node*> each_nodes; //each object
		glm::vec3 bg_color; //background color

		//Monte-Carlo raytrace
		void mc_ray_trace(const Ray &ray, int level, glm::vec3 &color, Node* last_node, bool direct_illug); 

		void input(FILE *pFile);	
		//area light
		vector<Node*> lights;
		void add_light(Node*); 

		//parameters
		int MAX_RECUR ; //ray tracing recursive max level 
		int AA_SAMPLE ; //anti-aliasing sampling rate
		int SHADOW_RAY_NUM ; //soft shadow ray nums
		int MC_MAX ; //Monte-Carlo iteration times, 0 means not do Monte-Carlo
		float DIRECT_ILLU_WEIGHTS;
};

#endif