// NOTE: This definition forces GLM to use radians (not degrees) for ALL of its
// angle arguments. The documentation may not always reflect this fact.
// YOU SHOULD USE THIS IN ALL FILES YOU CREATE WHICH INCLUDE GLM


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glew/glew.h"
#include <GL/glut.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "EasyBMP.h"

#include "Geometry.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Cube.h"
#include "node.h"
#include "Mesh.h"
#include "tests.h"
#include "camera.h"
//#include "ray_cast.h"
#include "material.h"
#include "ray_trace.h"


#define _CRT_SECURE_NO_WARNINGS

const float rotate_unit = 10.f * float(PI) / 180.0f;  // 10 degrees
float selected_colors[3] = {0.9f,1.f,1.f}; //white

//input variables
float eye_x, eye_y, eye_z;
float vdir_x, vdir_y, vdir_z;
float up_x, up_y, up_z, fovy;
//light pos. use for OpenGL preview
float lpos[4] = {0,8,0,0};
float lcol[4] = {1,1,1,1}; 
char flag[100];
char node_name[20]; 
float translation[3]; 
float input_rotation[3]; 
float scale[3]; 
float center[3]; 
char parent_name[20]; 
char shape_name[20]; 
char mesh_name[30]; 
float RGBA[4];

vector <Material> all_material;

//KdTree
int KDTREE_DEPTH = 12;

GLint attribute_lightpos;

// Attributes
unsigned int locationPos;
unsigned int locationCol;
unsigned int locationNor;
unsigned int locationTex;


// Uniforms
unsigned int unifModel;
unsigned int unifModelInvTr;
unsigned int unifViewProj;
unsigned int unifLightPos;
unsigned int unifLightColor;
unsigned int unifCameraPos;
unsigned int unifTexture;

GLuint texture_id;
//texture file
BMP in_bmp;
char texture_file[50];



// Needed to compile and link and use the shaders
unsigned int shaderProgram;

unsigned int windowWidth = 640;
unsigned int windowHeight = 480;

// Animation/transformation stuff
//clock_t old_time;
//float rotation = 0.0f;

std::string textFileRead(const char*);

void printLinkInfoLog(int);
void printShaderInfoLog(int);
void printGLErrorLog();

// Standard glut-based program functions
void init(void);
void resize(int, int);
void display(void);
void keypress(unsigned char, int, int);
void mousepress(int button, int state, int x, int y);
void cleanup(void);

//shader
void initShader();
void cleanupShader();

//opengl draw related 
void init_draw();
void DrawNode(Node*);
void DrawAllNode();
void UploadNode(Node*);
void change_node_col(Node*);

//tree related
vector<Node*>node_list;
vector<Node*>preorder_queue;
Node* cur_node; //current selected node
Node* root;
int preorder_cur_idx = 0;
Node* init_tree();
Node* get_next_node();
void init_preorder_queue();
void preorder_travese(Node *, bool);
bool all_nodes_removed();

//transform
void translate_root(Node*, glm::vec3);
void scale_root(Node* , glm::vec3);
void rotate_root(Node* , float , glm::vec3 );
void translate_sontree(Node* , glm::vec3);
void scale_sontree(Node* , glm::vec3 );
void rotate_sontree(Node* node, float degree, glm::vec3 mat);
//void update_sontree(Node*, glm::mat4);

void move_light(int type, float dis);

//ray tracer
void init_raytracer();
void ray_trace();

//single instance
Camera camera_ins; 
RayTrace raytrace_ins;

Node* init_tree()
{
	int root_index = 0;
	for(unsigned int i = 0; i < node_list.size(); ++i)
	{
		Node* tmp = node_list[i];
		if(strcmp(tmp->self_parent_name, "null") != 0) //not root
		{
			//find parent
			for(unsigned int j = 0; j < node_list.size(); ++j)
			{
				//is parent
				if(strcmp(node_list[j]->self_name, tmp->self_parent_name) == 0)
				{
					tmp->self_parent = node_list[j];
					tmp->UpTransmat();
					node_list[j]->add_son(node_list[i]);
					break;
				}
			}
		}
		else //root
		{
			root_index = i;	
		}
	}
	return node_list[root_index];
}

//cache the preorder queue, only create once!
void init_preorder_queue()
{
	preorder_travese(root, true);		
}

void preorder_travese(Node * node, bool is_init = false)
{
	if(!node)return;
	if(is_init)
		preorder_queue.push_back(node);
	for(unsigned int i = 0; i < node->son_list.size(); ++i)
	{
		preorder_travese(node->son_list[i], is_init);
	}
}

//1 if its father only has one son, return its father as next node
//2 else return its father's next son.
Node* get_remove_next_node(Node* node)
{
	if(node == root)
		return NULL;
	Node* parent = node->self_parent;
	int idx;
	for(unsigned int i = 0; i < parent->son_list.size(); ++i)
	{
		if(parent->son_list[i] == node)
		{
			idx = i;
			break;
		}
	}
	if(idx == parent->son_list.size() - 1)
	{
		return parent;
	}
	return parent->son_list[idx + 1];
}

Node* get_next_node()
{
	if(all_nodes_removed())
		return NULL;
	if(++preorder_cur_idx >= preorder_queue.size())	
		preorder_cur_idx = 0;
	Node* node = preorder_queue[preorder_cur_idx];
	printf("Get next node: %s\n", node->self_name);
	return node;
}


void recover_node_col(Node* node)
{	
	//root
	if(!node->self_geometry)
	{
		for(unsigned int i = 0; i < node->son_list.size(); ++i)
		{
			Node* son = node->son_list[i];
			if(son->self_geometry)
			{
				son->self_geometry->recover_colors();
				UploadNode(son);
				return;
			}
		}
	}
	node->self_geometry->recover_colors();
	UploadNode(node);
}

void change_node_col(Node* node)
{
	//root
	if(!node->self_geometry)
	{
		for(unsigned int i = 0; i < node->son_list.size(); ++i)
		{
			Node* son = node->son_list[i];
			if(son->self_geometry)
			{
				son->self_geometry->set_colors(selected_colors);
				UploadNode(son);
				return;
			}
		}
	}

	node->self_geometry->set_colors(selected_colors);
	UploadNode(node);
}

bool all_nodes_removed()
{
	return root == NULL; 
}

void on_select_next_node()
{
	if(all_nodes_removed())
		return;
	recover_node_col(cur_node);
	cur_node = get_next_node();
	change_node_col(cur_node);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	DrawAllNode();
	
	glutSwapBuffers();
}
	
void DrawAllNode()
{
	for(unsigned int i = 0; i< preorder_queue.size(); ++i)
	{
		DrawNode(preorder_queue[i]);
	}
}

void rotate_root(Node* node, float degree, glm::vec3 mat)
{
	printf("Rotate node: %s %f %f %f %f\n", node->self_name, degree, mat.x, mat.y, mat.z);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	node->Rotate(degree, mat, true);
	rotate_sontree(node, degree, mat);
	DrawAllNode();

    glutSwapBuffers();
}

void scale_root(Node* node, glm::vec3 mat)
{
	printf("Scale node: %s %f %f %f\n", node->self_name, mat.x, mat.y, mat.z);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	node->Scale(mat, true);
	scale_sontree(node, mat);
	DrawAllNode();

    glutSwapBuffers();
}

void translate_root(Node* node, glm::vec3 mat)
{
	printf("Translate node: %s %f %f %f\n", node->self_name, mat.x, mat.y, mat.z);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	node->Translate(mat);
	translate_sontree(node, mat);
	DrawAllNode();

    glutSwapBuffers();
}

void remove_node(Node * node, int level)
{
	if(node == NULL)
		return;
	printf("remove node-------------%s\n", node->self_name); 
	//printf("remove node-------------------%s %d\n", node->self_name, node->son_list.size());
	int son_len = node->son_list.size(); 
	for(int i = 0; i < son_len; ++i)
	{
		 remove_node(node->son_list[i], level + 1);	
	}
	
	for(unsigned int i = 0;i<node_list.size();++i)
	{
		if(node == node_list[i])
		{
			node_list.erase (node_list.begin()+i);
			break;
		}
	}
	for(unsigned int i = 0;i<preorder_queue.size();++i)
	{
		if(node == preorder_queue[i])
		{
			preorder_queue.erase (preorder_queue.begin()+i);
			break;
		}	
	}

	Node* parent = node->self_parent;
	//if has parenet and it is the root of the removed tree
	if(parent != NULL  && level == 0)
	{
		int son_len = parent->son_list.size(); 
		if(son_len == 1)
		{
			parent->son_list.clear();
		}
		else
		{
			for(int i = 0; i < son_len;++i)
			{
				if(node == parent->son_list[i])
				{
					parent->son_list.erase (parent->son_list.begin()+i);
					break;
				}	
			}
		}
	}
	if (node == root)
	{
		root = NULL;
		cur_node = NULL;
	}
	delete node;
	node = NULL;
}

void clear_tree(Node * node)
{
	if(!node)return;
	for(unsigned int i = 0; i < node->son_list.size(); ++i)
	{
		 clear_tree(node->son_list[i]);	
	}
	delete node;
	node = NULL;
}

bool load_mesh_obj(Geometry* mesh_obj, char mesh_name[])
{

	FILE * pFile;	
	char var[20];
	float v[3], vn[3], vt[3]; //vertex, normal, texture coordinate 
	int vi[3], ni[3], ti[3]; //vertex, normal, texture index 
	pFile = fopen(mesh_name,"r");
	if(pFile == NULL) 
	{
		printf("Can not open file:%s, Make sure the input files copied in this directory!\n", mesh_name);
		return false;
	}
	
	bool has_vn, has_vt;
	char slash, slash2;
	has_vn = has_vt = false;

	while(fscanf(pFile, "%s", var)!=EOF)
	{
		if(strcmp(var, "v") == 0)
		{
			fscanf(pFile, "%f %f %f", v, v+1, v+2);
			mesh_obj->add_vertex(v);
		}
		else if(strcmp(var, "vn") == 0)
		{
			fscanf(pFile, "%f %f %f", vn, vn+1, vn+2);
			mesh_obj->add_normal(vn);
			has_vn = true;
		}
		else if(strcmp(var, "vt") == 0)
		{
			fscanf(pFile, "%f %f", vt, vt+1);
			mesh_obj->add_texture(vt);
			has_vt = true;
		}
		else if(strcmp(var, "f") == 0) //face index
		{
			if(!has_vt && !has_vn) //no texture and normal
			{
				for(unsigned int i = 0; i < 3; ++i) //triangle
				{
					fscanf(pFile, "%d", vi+i);
					--vi[i]; 
				}
			}
			else if(!has_vt) //no texture
			{
				for(unsigned int i = 0; i < 3; ++i) //triangle
				{
					fscanf(pFile, "%d%c%c%d", vi+i, &slash, &slash2, ni+i);
					--vi[i]; 
					--ni[i];
				}
			}
			else if(!has_vn) //no normal
			{
				for(unsigned int i = 0; i < 3; ++i) //triangle
				{
					fscanf(pFile, "%d%c%d", vi+i, &slash, ti+i);
					--vi[i];
					--ti[i];
				}
			}
			else //has both texture and normal
			{
				for(unsigned int i = 0; i < 3; ++i) //triangle
				{
					fscanf(pFile, "%d%c%d%c%d", vi+i, &slash, ti+i, &slash2, ni+i);
					--vi[i];
					--ti[i];
					--ni[i];
				}
			}
			mesh_obj->add_indice(vi);
		}
	}
	//mesh_obj->display();
	mesh_obj->build_kdtree(KDTREE_DEPTH); //create kdtree 
	mesh_obj->init_tri_normals(); //create each triangle's normal
	 	
	return true;
}

void input_material(FILE *pFile)
{
	Material ma;
	ma.input(pFile);
	all_material.push_back(ma);
}

void create_new_node(FILE *pFile, char node_name[], float translation[], 
					 float input_rotation[], float scale[], float center[], char parent_name[], char shape_name[], char ma_name[])
{
	Node* new_node = new Node(shape_name);
	//find material
	for (vector<Material>::iterator iter = all_material.begin(); iter != all_material.end(); iter++)
	{
		if(strcmp(iter->name, ma_name) == 0)
		{
			new_node->ma = &(*iter);
			break;
		}
	}
	new_node->init_indata( node_name, translation, input_rotation, scale, center, parent_name, shape_name);
	
	if(strcmp(shape_name, "mesh") == 0)
	{
		//fscanf(pFile, "%s %s", flag, mesh_name);
		load_mesh_obj((new_node->self_geometry), mesh_name);
	}	
	if(strcmp(node_name, "light") == 0)
	{
		raytrace_ins.add_light(new_node);
		node_list.push_back(new_node);
	}
	else
	{
		node_list.push_back(new_node);
	}
}

bool input_file(int argc, char** argv)
{
	FILE * pFile;	
	if(argc <= 1)
	{
		printf("please input the file_name!\n");
		return false;
	}
	for(unsigned int i = 1;i<argc;++i)
	{
		//Input_file arguments: argv[2], argv[2], ...
		pFile = fopen(argv[i],"r");
		if(pFile == NULL) 
		{
			printf("Can not open file:%s, Make sure the input files copied in this directory!\n", argv[i]);
			return false;
		}
	}
	while(fscanf(pFile, "%s", flag)!=EOF)
	{
		if(strcmp(flag, "RESO") == 0)
			fscanf(pFile, "%d %d", &windowWidth, &windowHeight);
		else if(strcmp(flag, "EYEP") == 0)
			fscanf(pFile, "%f %f %f",&eye_x, &eye_y, &eye_z);
		else if(strcmp(flag, "VDIR") == 0)
			fscanf(pFile, "%f %f %f",&vdir_x, &vdir_y, &vdir_z);
		else if(strcmp(flag, "UVEC") == 0)
			fscanf(pFile, "%f %f %f",&up_x, &up_y, &up_z);
		else if(strcmp(flag, "FOVY") == 0)
			fscanf(pFile, "%f",&fovy);
		else if(strcmp(flag, "RAYTRACE") == 0)
		{
			raytrace_ins.input(pFile);
		}
		else if(strcmp(flag, "MAT") == 0)
		{
			input_material(pFile);
		}
		else if(strcmp(flag, "KDTREE") == 0)
		{
			fscanf(pFile, "%s %d", flag, &KDTREE_DEPTH);
		}
		else if(strcmp(flag, "NODE") == 0)
		{
			fscanf(pFile, "%s", node_name);
			fscanf(pFile, "%s %f %f %f", flag, translation, translation+1, translation+2);
			fscanf(pFile, "%s %f %f %f", flag, input_rotation, input_rotation+1, input_rotation+2);
			fscanf(pFile, "%s %f %f %f", flag, scale, scale+1, scale+2);
			fscanf(pFile, "%s %f %f %f", flag, center, center+1, center+2);
			fscanf(pFile, "%s %s", flag, parent_name);
			fscanf(pFile, "%s %s", flag, shape_name);
			if(strcmp(shape_name, "mesh") == 0)
			{
				fscanf(pFile, "%s %s", flag, mesh_name);
			}
			char ma[20];
			fscanf(pFile, "%s %s", flag, ma); 
			create_new_node(pFile, node_name, translation, input_rotation, scale, center, parent_name, shape_name, ma);
		}
	}
	return true;
}


int main(int argc, char** argv)
{
	srand(time(NULL));
	bool res = input_file(argc, argv);
	if(!res)
	{
		printf("input error");
		system("pause");
		return 0;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Scene Graph");

    glewInit();
	
    init();
	init_raytracer();
	init_draw();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keypress);
    glutMouseFunc(mousepress);
    glutIdleFunc(display);
    glutMainLoop();
	
    return 0;
}

void init()
{
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);

    initShader();
			
	//init node tree
	root = init_tree();
	if(root->self_geometry)
	{
		cur_node = root;
	}
	else
	{
		for(int i = 0; i < root->son_list.size(); ++i)
		{
			if(root->son_list[i]->self_geometry)
			{
				cur_node = root->son_list[i];
				break;
			}
		}
	}
	
	init_preorder_queue();
	for(unsigned int i = 0;i<preorder_queue.size();++i)
	{
		UploadNode(preorder_queue[i]);
	}
}



void initShader()
{
    // Read in the shader program source files
    std::string vertSourceS = textFileRead("shaders/diff.vert.glsl");
    const char *vertSource = vertSourceS.c_str();
    std::string fragSourceS = textFileRead("shaders/diff.frag.glsl");
    const char *fragSource = fragSourceS.c_str();

    // Tell the GPU to create new shaders and a shader program
    GLuint shadVert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shadFrag = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();

    // Load and compiler each shader program
    // Then check to make sure the shaders complied correctly
    // - Vertex shader
    glShaderSource    (shadVert, 1, &vertSource, NULL);
    glCompileShader   (shadVert);
    printShaderInfoLog(shadVert);
    // - Diffuse fragment shader
    glShaderSource    (shadFrag, 1, &fragSource, NULL);
    glCompileShader   (shadFrag);
    printShaderInfoLog(shadFrag);

    // Link the shader programs together from compiled bits
    glAttachShader  (shaderProgram, shadVert);
    glAttachShader  (shaderProgram, shadFrag);
    glLinkProgram   (shaderProgram);
    printLinkInfoLog(shaderProgram);

    // Clean up the shaders now that they are linked
    glDetachShader(shaderProgram, shadVert);
    glDetachShader(shaderProgram, shadFrag);
    glDeleteShader(shadVert);
    glDeleteShader(shadFrag);

    // Find out what the GLSL locations are, since we can't pre-define these
    locationPos    = glGetAttribLocation (shaderProgram, "vs_Position");
    locationNor    = glGetAttribLocation (shaderProgram, "vs_Normal");
    locationCol    = glGetAttribLocation (shaderProgram, "vs_Color");

    unifViewProj   = glGetUniformLocation(shaderProgram, "u_ViewProj");
    unifModel      = glGetUniformLocation(shaderProgram, "u_Model");
    unifModelInvTr = glGetUniformLocation(shaderProgram, "u_ModelInvTr");
    unifLightPos = glGetUniformLocation(shaderProgram, "u_LightPos");
    unifLightColor = glGetUniformLocation(shaderProgram, "u_LightColor");
    unifCameraPos = glGetUniformLocation(shaderProgram, "u_CameraPos");
    
    printGLErrorLog();
}

void cleanup()
{
    glDeleteProgram(shaderProgram);
	clear_tree(root);
	node_list.clear();
	preorder_queue.clear();
}

void init_raytracer()
{
	//init camera for ray_tracing
	camera_ins.set_position(eye_x,eye_y,eye_z);
	camera_ins.set_direction(vdir_x, vdir_y, vdir_z);
	camera_ins.set_up_vec(up_x, up_y, up_z);
	camera_ins.set_fov(fovy / 2.0f);
	camera_ins.w = windowWidth;
	camera_ins.h = windowHeight;

    resize(windowWidth, windowHeight);
}

void ray_trace()
{
	printf("\n ---------------begin ray_tracing! ---------------\n");	
	raytrace_ins.enter(camera_ins, preorder_queue);
}

void keypress(unsigned char key, int x, int y)
{
	if(all_nodes_removed() && key != 'q')
	{
		printf("All nodes have been removed! \n");
		return;
	}
    switch (key) {
    case 'q':
        cleanup();
        exit(0);
        break;
    case 'n':
		on_select_next_node();
		break;
	case 'a':
		translate_root(cur_node, glm::vec3(-0.5, 0, 0));
		break;
	case 'd':
		translate_root(cur_node, glm::vec3(0.5, 0, 0));
		break;
	case 'w':
		translate_root(cur_node, glm::vec3(0, 0.5, 0));
		break;
	case 's':
		translate_root(cur_node, glm::vec3(0, -0.5, 0));
		break;
	case 'e':
		translate_root(cur_node, glm::vec3(0, 0, 0.5));
		break;
	case 'r':
		translate_root(cur_node, glm::vec3(0, 0, -0.5));
		break;
	case 'x':
		scale_root(cur_node, glm::vec3(1.5f, 1.f, 1.f));
		break;
	case 'X':
		scale_root(cur_node, glm::vec3(0.5f, 1.f, 1.f));
		break;
	case 'y':
		scale_root(cur_node, glm::vec3(1.f, 1.5f, 1.f));
		break;
	case 'Y':
		scale_root(cur_node, glm::vec3(1.f, 0.5f, 1.f));
		break;
	case 'z':
		scale_root(cur_node, glm::vec3(1.f, 1.f, 1.5f));
		break;
	case 'Z':
		scale_root(cur_node, glm::vec3(1.f, 1.f, 0.5f));
		break;
	case 'j': //x
		rotate_root(cur_node, rotate_unit, glm::vec3(1.f, 0, 0));
		break;
	case 'J': //x
		rotate_root(cur_node, -rotate_unit, glm::vec3(1.f, 0, 0));
		break;
	case 'k': //y
		rotate_root(cur_node, rotate_unit, glm::vec3(0, 1.f, 0));
		break;
	case 'K': //y
		rotate_root(cur_node, -rotate_unit, glm::vec3(0, 1.f, 0));
		break;
	case 'l': //z
		rotate_root(cur_node, rotate_unit, glm::vec3(0, 0, 1.f));
		break;
	case 'L': //z
		rotate_root(cur_node, -rotate_unit, glm::vec3(0, 0, 1.f));
		break;
	case 'f': 
		move_light(0, 0.5);
		break;
	case 'F': 
		move_light(0, -0.5);
		break;
	case 'g': 
		move_light(1, 0.5);
		break;
	case 'G': 
		move_light(1, -0.5);
		break;
	case 'h': 
		move_light(2, 0.5);
		break;
	case 'H': 
		move_light(2, -0.5);
		break;
	case 'p': 
		ray_trace();
		break;
	case 'u': //remove node
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(all_nodes_removed())
			break;
		Node* next_node = get_remove_next_node(cur_node);
		if(next_node != NULL)
		{
			printf("remove next node: %s\n", next_node->self_name);
		}
		remove_node(cur_node, 0);
		if(all_nodes_removed())
		{
			cur_node = NULL;
			root = NULL;
			printf("All nodes have been removed! \n");
			glutSwapBuffers();
		}
		else
		{
			printf("After remove, Left node:\n");
			for(unsigned int i = 0;i<preorder_queue.size();++i)
			{
				printf("%s ", preorder_queue[i]->self_name);
			}
			printf("\n");
			cur_node = next_node; 
			change_node_col(cur_node);
			DrawAllNode();
			glutSwapBuffers();
		}
		break;
    }

    glutPostRedisplay();
}

void mousepress(int button, int state, int x, int y)
{
    // Put any mouse events here
}

void init_draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//initial select the root node	
	change_node_col(cur_node);
	DrawAllNode();

    glutSwapBuffers();
    printGLErrorLog();
}

//recursive
void translate_sontree(Node* node, glm::vec3 mat)
{
	for(unsigned int i = 0; i < node->son_list.size(); ++i)
	{
		Node* son = node->son_list[i];
		son->Translate(mat);
		printf("Translate son: %s\n", son->self_name); 
		translate_sontree(son, mat);
	}
}

//recursive
void scale_sontree(Node* node, glm::vec3 mat)
{
	for(unsigned int i = 0; i < node->son_list.size(); ++i)
	{
		Node* son = node->son_list[i];
		son->Scale(mat, false);
		printf("scale son: %s\n", son->self_name); 
		scale_sontree(son, mat);
	}
}

//recursive
void rotate_sontree(Node* node, float degree, glm::vec3 mat)
{
	for(unsigned int i = 0; i < node->son_list.size(); ++i)
	{
		Node* son = node->son_list[i];
		son->Rotate(degree, mat, false);
		printf("rotate son: %s\n", son->self_name); 
		rotate_sontree(son, degree, mat);
	}
}

void move_raytracer_light()
{
	//raytrace_ins.lpos_list[0] = glm::vec3(lpos[0], lpos[1], lpos[2]);
}

void move_light(int type, float dis)
{
	if(type >= 3) return;
	lpos[type] += dis;
	printf("move light coordinate: %f %f %f\n", lpos[0], lpos[1], lpos[2]);
	glUniform4fv(unifLightPos, 1, lpos);
	
	//update raytracer
	move_raytracer_light();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	DrawAllNode();

	glutSwapBuffers();	
	printGLErrorLog();
}

void UploadNode(Node* node)
{
	Geometry * geo_ins = node->self_geometry;
	if(!geo_ins)
	{
		//printf("UploadNode: Geometry is NULL %s\n", node->self_name);
		return;
	}
	const int VERTICES = geo_ins->getVertexCount();
	const int IndexCount = geo_ins->getIndexCount();
	const int NormalsCount = geo_ins->getNormals().size();
	//const int TextureCount = geo_ins->getTextureCount();

	//printf("%s Index Count£º%d\n", node->self_name, geo_ins->getIndexCount());
	//printf("%s Normals Count£º%d\n", node->self_name, geo_ins->getNormals().size());

	glm::vec3* geo_vertices = new glm::vec3[VERTICES]; 
	glm::vec3* geo_colors = new glm::vec3[VERTICES]; 
	glm::vec3* geo_normals = new glm::vec3[NormalsCount]; 
	unsigned int* geo_indexs = new unsigned int[IndexCount];
	//glm::vec2 * geo_texture = new glm::vec2[TextureCount]; 

	for(int i = 0; i < VERTICES; ++i)
	{
		geo_vertices[i] = geo_ins->getVertices()[i];
//		geo_colors[i] = geo_ins->getColors()[i];
		//printf("vertice xyz:%f %f %f\n", tmp.x, tmp.y, tmp.z);
	}
	for(int i = 0; i < IndexCount; ++i) 
	{
		geo_indexs[i] = geo_ins->getIndices()[i];
	}
	//printf("%s sizeof index£º%d\n", node->self_name, sizeof(geo_indexs)); 
	
	for(int i = 0; i < NormalsCount; ++i) 
	{
		geo_normals[i] = geo_ins->getNormals()[i];
	}
	
//	for(int i = 0; i < TextureCount; ++i) 
//	{
//		geo_texture[i] = geo_ins->getTexture()[i];
//	}

	glGenBuffers(1, &(node->vbo_vertices));
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*VERTICES, geo_vertices, GL_STATIC_DRAW);
	delete[] geo_vertices;

	glGenBuffers(1, &node->vbo_colors);
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*VERTICES, geo_colors, GL_STATIC_DRAW);
	delete[] geo_colors;

	glGenBuffers(1, &(node->vbo_normals));
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*NormalsCount, geo_normals, GL_STATIC_DRAW);
	delete[] geo_normals;

	glGenBuffers(1, &(node->vbo_index));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->vbo_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(geo_indexs) * IndexCount, geo_indexs, GL_STATIC_DRAW);
	delete[] geo_indexs;	
}

void DrawNode(Node * node)
{
	if(!node->self_geometry) return;
    glm::mat4 model = glm::mat4();
	model = node->self_transmat; 

	//glBindBuffer(GL_ARRAY_BUFFER, node->vbo_vertices);
	glEnableVertexAttribArray(locationPos);
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_vertices);
	glVertexAttribPointer( locationPos, 3, GL_FLOAT,   GL_FALSE,  0,   0 );

	glEnableVertexAttribArray(locationCol);
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_colors);
	glVertexAttribPointer( locationCol, 3, GL_FLOAT,   GL_FALSE,  0,   0);

	glEnableVertexAttribArray(locationNor);
	glBindBuffer(GL_ARRAY_BUFFER, node->vbo_normals);
	glVertexAttribPointer( locationNor, 3, GL_FLOAT,   GL_FALSE,  0,   0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->vbo_index);
	int size;  
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	//printf("Draw size:%d\n", size);

	//transform
	glUniformMatrix4fv(unifModel, 1, GL_FALSE, &model[0][0]);
	// Also upload the inverse transpose for normal transformation
	const glm::mat4 modelInvTranspose = glm::inverse(glm::transpose(model));
	glUniformMatrix4fv(unifModelInvTr, 1, GL_FALSE, &modelInvTranspose[0][0]);
	
	glDrawElements(GL_TRIANGLES, size/sizeof(unsigned int), GL_UNSIGNED_INT, 0); 

	glDisableVertexAttribArray(locationPos);
	glDisableVertexAttribArray(locationCol);
	glDisableVertexAttribArray(locationNor);
}

void resize(int width, int height)
{
    // Set viewport
    glViewport(0, 0, width, height);

    // Get camera information
    // Add code here if you want to play with camera settings/ make camera interactive.
	float fovy_radian = fovy * PI / 180.0f;
    glm::mat4 projection = glm::perspective(fovy_radian, width / (float) height, 0.1f, 100.0f);
    glm::mat4 camera = glm::lookAt(glm::vec3(eye_x, eye_y, eye_z), 
		glm::vec3(vdir_x, vdir_y, vdir_z), glm::vec3(up_x, up_y, up_z));
    projection = projection * camera;
		
    glUseProgram(shaderProgram);

	//initial the light 
	glUniform4fv(unifLightPos, 1, lpos);
	glUniform4fv(unifLightColor, 1, lcol);

	//pass to shader the camera position, to create the specular light
	float camera_pos[4] = {eye_x, eye_y, eye_z, 1};
	glUniform4fv(unifCameraPos, 1, camera_pos);

    // Upload the projection matrix, which changes only when the screen or
    // camera changes
    glUniformMatrix4fv(unifViewProj, 1, GL_FALSE, &projection[0][0]);

    glutPostRedisplay();
}


std::string textFileRead(const char *filename)
{
    // http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    std::ifstream in(filename, std::ios::in);
    if (!in) {
        std::cerr << "Error reading file" << std::endl;
        throw (errno);
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void printGLErrorLog()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << error << ": ";
        const char *e =
            error == GL_INVALID_OPERATION             ? "GL_INVALID_OPERATION" :
            error == GL_INVALID_ENUM                  ? "GL_INVALID_ENUM" :
            error == GL_INVALID_VALUE                 ? "GL_INVALID_VALUE" :
            error == GL_INVALID_INDEX                 ? "GL_INVALID_INDEX" :
            "unknown";
        std::cerr << e << std::endl;

        // Throwing here allows us to use the debugger stack trace to track
        // down the error.
#ifndef __APPLE__
        // But don't do this on OS X. It might cause a premature crash.
        // http://lists.apple.com/archives/mac-opengl/2012/Jul/msg00038.html
        throw;
#endif
    }
}

void printLinkInfoLog(int prog)
{
    GLint linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) {
        return;
    }
    std::cerr << "GLSL LINK ERROR" << std::endl;

    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);
        std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
        delete[] infoLog;
    }
    // Throwing here allows us to use the debugger to track down the error.
    throw;
}

void printShaderInfoLog(int shader)
{
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) {
        return;
    }
    std::cerr << "GLSL COMPILE ERROR" << std::endl;

    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
        std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
        delete[] infoLog;
    }
    // Throwing here allows us to use the debugger to track down the error.
    throw;
}
