#ifndef NODE_H 
#define NODE_H 

#include <vector>
#include <cstdio>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "string.h"

#include "Geometry.h"
#include "material.h"
#include "Sphere.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Mesh.h"

class Node 
{
public:
	Node();
	Node(char shape[]);
	~Node();
	
	//input data
    char self_name[20]; 
	float self_translation[3]; 
	float self_rotation[3]; 
	float self_scale[3]; 
	float self_center[3]; 
	char self_parent_name[20]; 
	char self_shape[20]; 

	//node data
	Geometry* self_geometry;
	Node* self_parent; 
	vector<Node*>son_list;

	//ray tracing
	Material* ma;
	bool has_raytrace; //avoid repeat raytrace recursively

	//shader
	unsigned int vbo_vertices;
	unsigned int vbo_colors;
	unsigned int vbo_index;
	unsigned int vbo_normals;
	unsigned int vbo_texture;

	//funcs
	void display();
	void add_son(Node* );
	void build_geometry(char shape[]);
	glm::mat4 GetParentMat();
	void Transform(glm::mat4 model);
	void Node::UpTransmat();

	//transform
	void Translate(glm::vec3 mat);
	void Scale(glm::vec3 mat, bool);
	void Rotate(float , glm::vec3, bool );
	glm::vec4 real_center;
	
	//init
	void init_indata(char name[], float translation[], float rotation[], float scale[], 
		float center[], char parent_name[], char shape_name[]); 
	void init_center();
	void init_transmat();
	void TranslateBackToOrigin();
	void TranslateToCenter();
	glm::mat4 self_transmat; //all transformation matrix
	glm::mat4 self_translate_mat; //translation matrix, for changing the center

	//light node
	bool is_light();
};
#endif
