#include "material.h"

Material::Material(){}

Material::~Material() {}

void Material::input(FILE *pFile)
{
		char flag[20];
		fscanf(pFile, "%s", name); 
		fscanf(pFile, "%s %f %f %f", flag, &diffuse.x, &diffuse.y, &diffuse.z);
		fscanf(pFile, "%s %f %f %f", flag, &specular.x, &specular.y, &specular.z);
		fscanf(pFile, "%s %f", flag, &expo);
		fscanf(pFile, "%s %f", flag, &refraction);
		fscanf(pFile, "%s %d", flag, &mirr);
		fscanf(pFile, "%s %d", flag, &tran);		
		fscanf(pFile, "%s %f", flag, &emit);		

		//diffuse material
		if(!can_reflect() && !can_refract())
		{
			absorb = 1.f - std::max(diffuse.x, std::max(diffuse.y, diffuse.z));
		}
		else
		{
			absorb = 0.f;
		}
}

bool Material::can_reflect()
{
	return mirr == 1;
}

bool Material::can_refract()
{
	return tran == 1; 
}

bool Material::is_diffuse()
{
	return mirr == 0 && tran == 0; 
}