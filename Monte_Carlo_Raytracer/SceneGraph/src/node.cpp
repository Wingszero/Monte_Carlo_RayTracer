#include "node.h"

Node::Node(char shape[])
{
	build_geometry(shape);
	ma = NULL;
	has_raytrace = false;
}

Node::~Node()
{
	printf("delete node:%s\n", self_name);
	self_parent = NULL;
	ma = NULL;
	son_list.clear();
	if(self_geometry)
	{
		delete self_geometry;
		self_geometry = NULL;
	}
}

void Node::init_indata(char name[], float translation[], float rotation[], float scale[], 
		float center[], char parent_name[], char shape_name[])
{
	strcpy(self_name, name);
	memcpy(self_translation, translation, sizeof(translation) * 3);
	memcpy(self_rotation, rotation, sizeof(rotation) * 3);
	for(int i = 0; i < 3; i++)
	{
		self_rotation[i] *= Radian; 
	}
	memcpy(self_scale, scale, sizeof(scale) * 3);
	memcpy(self_center, center, sizeof(center) * 3);
	strcpy(self_parent_name, parent_name);
	strcpy(self_shape, shape_name);
	
	self_parent = NULL;	
	real_center = glm::vec4(self_center[0], self_center[1], self_center[2], 1);
	init_transmat();
	init_center();

	//compatible with the old input
	float colors[3] = {1,1,1};
	if(self_geometry)
	{
		self_geometry->init_colors(colors);
		self_geometry->init_normals();
	}
}

void Node::init_center()
{
	real_center = self_translate_mat * real_center;
	//printf("node real center: %f %f %f\n", real_center[0], real_center[1], real_center[2]);
}

void Node::init_transmat()
{
	self_transmat = glm::mat4(); 
	//transform order:
	// matrix: translate*rotate*scale, 
	//the real transform order is opposite: scale first, then rot, trans last.

	//rotate
	self_transmat *= 
		((glm::rotate(glm::mat4(1.0f), self_rotation[0], glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), self_rotation[1], glm::vec3(0, 1, 0))* 
		glm::rotate(glm::mat4(1.0f), self_rotation[2], glm::vec3(0, 0, 1))) ); 

	//scale
	self_transmat *= glm::scale(glm::mat4(1.0f), 
		glm::vec3(self_scale[0], self_scale[1], self_scale[2])) ; 

	//translate (global coordinate)
	self_transmat = (glm::translate(glm::mat4(1.0f), 
		glm::vec3(self_translation[0], self_translation[1], self_translation[2])) ) * self_transmat;

	//tranlation matrix
	self_translate_mat = glm::translate(glm::mat4(1.0f), 
		glm::vec3(self_translation[0], self_translation[1], self_translation[2])); 
}


void Node::build_geometry(char shape[])
{
	if(strcmp(shape, "cube") == 0)
	{
		self_geometry = new Cube();
	}
	else if(strcmp(shape, "sphere") == 0)
	{
		self_geometry = new Sphere(); 
	}
	else if(strcmp(shape, "cylinder") == 0)
	{
		self_geometry = new Cylinder();
	}
	else if(strcmp(shape, "mesh") == 0)
	{
		self_geometry = new Mesh();
	}
	else if(strcmp(shape, "null") == 0)
	{
		self_geometry = NULL; 
	}
	else
	{
		printf("Invalid input geometry type: %s!\n", shape);
		return;
	}
}

void Node::display()
{
	printf("name %s \n", self_name);
	printf("translation %f %f %f\n:", self_translation[0], self_translation[1], self_translation[2]);
	//printf("RGBA %f %f %f\n\n", self_RGBA[0], self_RGBA[1], self_RGBA[2]);
}

void Node::add_son(Node* son)
{
	son_list.push_back(son);
}

glm::mat4 Node::GetParentMat()
{
	if(!self_parent)
		return glm::mat4();
	return self_parent->self_transmat;
}

void Node::Transform(glm::mat4 model) 
{
	self_transmat = model * self_transmat;
}

void Node::Translate(glm::vec3 mat)
{
	glm::mat4 real_mat = glm::translate(glm::mat4(1.0f), mat); 
	self_transmat = real_mat * self_transmat; 
	self_translate_mat = real_mat * self_translate_mat; 
	real_center = real_mat  * real_center;
}

void Node::TranslateBackToOrigin()
{
	glm::vec3 mat(-real_center.x, -real_center.y, -real_center.z);
	//printf("node real center: %s %f %f %f\n", self_name, real_center[0], real_center[1], real_center[2]);
	self_transmat = glm::translate(glm::mat4(1.0f), mat) * self_transmat; 
}

void Node::TranslateToCenter()
{
	glm::vec3 mat(real_center.x, real_center.y, real_center.z);
	//printf("node real center: %s %f %f %f\n", self_name, real_center[0], real_center[1], real_center[2]);
	self_transmat = glm::translate(glm::mat4(1.0f), mat) * self_transmat; 
}

void Node::Scale(glm::vec3 mat, bool is_root)
{
	//TranslateBackToOrigin();

	if(is_root)
		self_transmat *= glm::scale(glm::mat4(1.0f), mat);
	else
		self_transmat = glm::scale(glm::mat4(1.0f), mat) * self_transmat; 

	//if(!is_root)
		//TranslateToCenter();
}

void Node::Rotate(float degree, glm::vec3 mat, bool is_root)
{
	//if(!is_root)
		//TranslateBackToOrigin();
	
	if(is_root)
		self_transmat *= glm::rotate(glm::mat4(1.0f), degree, mat) ; 
	else
		self_transmat = glm::rotate(glm::mat4(1.0f), degree, mat) * self_transmat; 

	//if(!is_root)
		//TranslateToCenter();
}

void Node::UpTransmat()
{
	if(self_parent != NULL)
	{
		self_transmat = self_parent->self_transmat * self_transmat;
		self_translate_mat = self_parent-> self_translate_mat * self_translate_mat;
		real_center = self_parent->self_translate_mat * real_center;
	}
}

bool Node::is_light()
{
	return strcmp(self_name, "light") == 0;
}