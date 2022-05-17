#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenarán todas las entidades en la escena y contendrá metodos para facilitar el trabajo en ellas
//Usa el patrón singleton
#include "utils.h"
#include "includes.h"
#include "entity.h"
#include "entitymesh.h"
#include "camera.h"
class Scene
{

private:
	Scene() { /*........*/ };
public:
	std::vector<Entity*> entities;

	static Scene& getInstance()
	{
		static Scene instance;
		return instance;
	}
	void addEntityMesh(const char* meshName, const char* texName, const char* shader, Vector4 color, Matrix44 model);
	void eraseEntity(int i);
	//Retorna las entidades en camara para que sean renderizadas
	std::vector<Entity*> getEntitiesInView(Camera* cam);
};


#endif