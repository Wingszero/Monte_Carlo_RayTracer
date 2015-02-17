#ifndef MATERIAL_H 
#define MATERIAL_H 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <fstream>

class Material 
{
public:
    Material();
    virtual ~Material();
	void input(FILE *pFile);
	bool can_reflect();
	bool can_refract();
	bool is_diffuse();

	char name[20];
	glm::vec3 diffuse;
	glm::vec3 specular;
	float expo;
	float refraction;
	int mirr;
	int tran;
	float absorb;
	float emit;
   
};

#endif
