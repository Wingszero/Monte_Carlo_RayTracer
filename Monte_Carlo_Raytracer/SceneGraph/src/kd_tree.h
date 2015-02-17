#ifndef KDTREE_H 
#define KDTREE_H 

#include "bv.h"
#include "triangle.h"
#include <vector>
#include <algorithm>
#include <time.h>
 
static int MAX_LEVEL = 12;
const int MIN_TRIS_NUM = 10; //if a node's trianges less than this will not be divided anymore

class KdNode 
{
public:
    KdNode(); 
    KdNode(const vector<Triangle*>& in_tris, BV* pa_box, float mid, int dim, bool is_left); 
    virtual ~KdNode();
		
	KdNode* left;
	KdNode* right;

	BV *box; //bounding box of triangles
	vector<Triangle*>tris; //list of triangle

	int divide_node(int lev); //base on midpoint
	float find_mid(int dim); //fine midean to divide the node
	bool search(const Ray &ray, vector<KdNode*>&nodes); //find which node the ray will hit
	
	int level; //for debug
	void print();
};

class KdTree 
{
public:
    KdTree();
    KdTree(int max_depth);
    virtual ~KdTree();
	KdNode* root;
	int level;

	KdNode* build_tree(vector<glm::vec3>&vertices_, vector<unsigned int>&indices_); //input data from mesh
	bool search(const Ray &ray, vector<KdNode*>&nodes); //nodes contain the Kd-nodes which can be hit by ray
};
	
#endif
