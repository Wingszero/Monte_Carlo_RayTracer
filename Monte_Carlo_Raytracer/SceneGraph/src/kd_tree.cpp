#include "kd_tree.h"
#include <algorithm>

KdNode::KdNode()
{
	box = NULL;
	left = NULL;
	right = NULL;
}

KdNode::KdNode(const vector<Triangle*>& in_tris, BV* pa_box, float mid, int dim, bool is_left) //for son treenode
{
	box = new BV();
	box->min_pt = pa_box->min_pt;
	box->max_pt = pa_box->max_pt;
	switch(dim)
	{
	case 0:
		if(is_left)
			box->max_pt.x = mid;
		else
			box->min_pt.x = mid;
		break;
	case 1:
		if(is_left)
			box->max_pt.y = mid;
		else
			box->min_pt.y = mid;
		break;
	case 2:
		if(is_left)
			box->max_pt.z = mid;
		else
			box->min_pt.z = mid;
		break;
	}
	tris = in_tris;
	left = NULL;
	right = NULL;
}

KdNode::~KdNode() 
{
	tris.clear();
	delete box;
}

struct sort_cmp
{
	sort_cmp(int dim)
	{
		this->dim = dim;
	}
	bool operator() (Triangle *a, Triangle *b)
	{
		return a->min_pt[dim] < b->min_pt[dim];
	}
	int dim;
};

inline float quick_select_mid_pt(float a[], int n, int k)
{
	int index = rand()% n;
	float pivot = a[index]; 
	float *l = new float[n];
	float *r = new float[n];
	int ln,rn;
	ln = rn = 0;
	for(int i = 0; i < n; ++i)
	{
		if(a[i] < pivot)
			l[ln++] = a[i]; 
		else if(a[i] > pivot)
			r[rn++] = a[i]; 
	}
	if(k < ln) return quick_select_mid_pt(l,  ln, k); 
	if(k > n - rn) return quick_select_mid_pt(r,  rn, k - (n - rn)); 
	delete[] l;
	delete[] r;
	return pivot;
}

//slower than library sort, need speed up
inline float quick_select_mid(vector<Triangle*>&tris, int k, int dim) 
{
	int n = tris.size();
	int index = rand() % n;
	float pivot = tris[index]->min_pt[dim];
	float *l = new float[n];
	float *r = new float[n];
	int ln,rn;
	ln = rn = 0;
	for(int i = 0; i < n; ++i)
	{
		float x = tris[i]->min_pt[dim]; 
		if(x < pivot)
			l[ln++] = x; 
		else if(x > pivot)
			r[rn++] = x; 
	}
	if(k <= ln) return quick_select_mid_pt(l,  ln, k); 
	if(k > n - rn) return quick_select_mid_pt(r,  rn, k - (n - rn)); 
	delete[] l;
	delete[] r;
	return pivot;
}

//slower than library sort, need speed up
inline float quick_select_mid2(vector<Triangle*>&tris, int k, int dim, int st, int end)
{
	if(end <= st)
		return tris[end]->min_pt[dim];

	int n = end - st + 1; 
	float pivot = tris[rand() % n]->min_pt[dim];
	int i,j;
	i = st, j = end;
	while(i < j)
	{
		while(i < end && tris[i]->min_pt[dim] <= pivot)
			++i;
		while(j > st && tris[j]->min_pt[dim] >= pivot)
			--j;
		if(i < j) 
			swap(tris[i], tris[j]);
	}
	if(i - st > k) //num of smaller
		return quick_select_mid2(tris, k, dim, st, i - 1); 
	if(end - j > n - k) //num of bigger
		return quick_select_mid2(tris, k, dim, j + 1, end); 
	return pivot;
}


float KdNode::find_mid(int dim)
{
	sort(tris.begin(), tris.end(), sort_cmp(dim));
	return tris[tris.size()/2]->min_pt[dim];
}

int KdNode::divide_node(int lev)
{
	level = lev;
	int n = tris.size();
	if(lev >= MAX_LEVEL || n <= MIN_TRIS_NUM) return lev;	

	int dim = box->get_longest_axis(); 
	float mid = find_mid(dim);

	vector<Triangle*>left_tris;
	vector<Triangle*>right_tris;
	
	//if overlap triangles of left&right node reaches to this number, no more divide 
	int max_overlap = n / 4;  
	//overlap on left and right node
	int match = 0; 
	for(int i = 0; i < n; ++i) //each triangle	
	{
		bool l,r; 
		l = r = false; //if insert into left/right sub-tree
		float tar;
		Triangle* tri = tris[i];
		for(int j = 0; j < 3; ++j) //each triangle point
		{
			if(l && r)break;
			switch(dim)
			{
			case 0:
				tar = tri->p[j].x; 
				break;
			case 1:
				tar = tri->p[j].y; 
				break;
			case 2:
				tar = tri->p[j].z; 
				break;
			}
			bool smaller = (tar <= mid);
			l |= smaller;
			r |= !smaller;
		}
		//a triangle both in left&right tree, and overlap each other over 50%. 
		if(l && r && ++match >= max_overlap) return lev;
		if(l) left_tris.push_back(tri);
		if(r) right_tris.push_back(tri);
	}

	left = new KdNode(left_tris, box, mid, dim, true);
	right = new KdNode(right_tris, box, mid, dim, false);
	return std::max(left->divide_node(level + 1), right->divide_node(level + 1));

	/*
	if(level == 0)
	{
		printf("left box min: %f %f %f\n", left->box->min_pt.x, left->box->min_pt.y, left->box->min_pt.z);
		printf("left box max: %f %f %f\n", left->box->max_pt.x, left->box->max_pt.y, left->box->max_pt.z);
		printf("\n");
	}
	*/
}

bool KdNode::search(const Ray &ray, vector<KdNode*>&nodes) //find which node the ray will hit
{
	if(!box->aabb_intersect_test(ray))
		return false;

	bool res = false;
	if(left && left->search(ray, nodes))
		res = true;
	if(right && right->search(ray, nodes))
		res = true;

	if(!res) //is leaf, insert current node and return
		nodes.push_back(this);
	return true;
}

void KdNode::print()
{
	printf("\n");
	printf("level: %d\n", level);
	for(int i = 0; i < tris.size(); ++i)
	{
		printf("%d ", tris[i]->index);
	}
}

//------------KdTree-------------------------------
KdTree::KdTree(){}
KdTree::KdTree(int max_depth)
{
	MAX_LEVEL = max_depth;
}

KdTree::~KdTree() {}

bool KdTree::search(const Ray &ray, vector<KdNode*>&nodes)
{
	return root->search(ray, nodes);	
}

KdNode* KdTree::build_tree(vector<glm::vec3>&vertices_, 
						   vector<unsigned int>&indices_)
{
	vector<Triangle*>tris;
	//transder indices to Triangle obj
	for(int i = 0; i < indices_.size(); i += 3)
	{
		glm::vec3 p0,p1,p2;
		p0 = vertices_[indices_[i]];
		p1 = vertices_[indices_[i+1]];
		p2 = vertices_[indices_[i+2]];
		Triangle* tri = new Triangle(p0,p1,p2,i/3);
		tris.push_back(tri);	
	}
	root = new KdNode();
	root->tris = tris;
	root->box = new BV(vertices_);

	clock_t newtime, oldtime;
	oldtime = clock();

	printf("Mesh triangles num %d\n", tris.size());
	printf("KdTree start building...\n");

	level = root->divide_node(0);
	printf("KdTree finish building: \n");
	printf("Depth: %d\n", level);
	
	newtime = clock();
	float usetime = static_cast<float>(newtime - oldtime) / static_cast<float>(CLOCKS_PER_SEC);
	printf("Use time %.2f secs.\n", usetime);
	printf("-------------------------------------------------------\n");

	return root;
}
