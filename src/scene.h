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
<<<<<<< Updated upstream
=======
public:

	//Singleton
	static Scene* instance;

	//General features
	Vector3 ambient_light;
	Camera* main_camera;

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	Entity* main_character;
	Entity* monster;
	std::vector<Entity*> objects;
	std::vector<Entity*> lights;
	std::vector<Entity*> sounds;

	//Scene triggers
	bool entity_trigger; //Triggers if an entity has changed his visibility or a visible entity has changed its model.
	bool shadow_visibility_trigger; //Triggers changes in shadow casting or light visibility for lights that cast shadows.

	//Methods
	Scene();
	void clear();
	void addEntity(Entity*);
	void load();

>>>>>>> Stashed changes

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