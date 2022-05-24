#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenarán todas las entidades en la escena y contendrá metodos para facilitar el trabajo en ellas
//Usa el patrón singleton

#pragma once
#include "utils.h"
#include "includes.h"
#include "entity.h"
#include "camera.h"
#include "shader.h"

//Forward declaration
class FBO;

using namespace std;

class Scene
{
public:

	//Singleton
	static Scene* instance;

	//General features
	string filename;
	Vector3 ambient_light;
	Camera* main_camera;
	Shader* shader;

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	MainCharacterEntity* main_character;
	MonsterEntity* monster;
	vector<ObjectEntity*> objects;
	vector<LightEntity*> lights;
	vector<SoundEntity*> sounds;

	//Counters
	int num_objects;
	int num_lights;
	int num_shadows;

	//Scene properties
	bool show_atlas;
	int atlas_scope;

	//Scene triggers


	//Constructor
	Scene();

	//Entity methods
	void clear();
	void addEntity(Entity* entity);
	void removeEntity(Entity* entity);
	Vector3 testCollisions(Vector3 currPos, Vector3 nexPos, float elapsed_time);

	//JSON methods
	bool load(const char* scene_filepath);
	bool save();

};


#endif