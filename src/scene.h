#ifndef SCENE_H
#define SCENE_H
//En esta clase se almacenar�n todas las entidades en la escena y contendr� metodos para facilitar el trabajo en ellas
//Usa el patr�n singleton

#pragma once //Import things once
#include "utils.h"
#include "includes.h"
#include "entity.h"
#include "entitymesh.h"
#include "camera.h"
#include "game.h"


class Scene
{
public:

	//Singleton
	static Scene* instance;

	//General features
	Vector3 ambient_light;
	Camera* main_camera;
	Shader* shader;

	//Scene shadows
	FBO* fbo; //Frame Buffer Object
	Texture* shadow_atlas; //Shadow map of the lights of the scene

	//Entities
	MainCharacterEntity* main_character;
	MonsterEntity* monster;
	std::vector<ObjectEntity*> objects;
	std::vector<LightEntity*> lights;
	std::vector<SoundEntity*> sounds;

	//Counters
	int num_objects;
	int num_lights;

	//Scene triggers
	bool camera_trigger; //Triggers if the camera has moved in the space.

	//Methods
	Scene();
	void clear();
	void load();
	void addEntity(Entity* entity);
	void AllocateMemory()


};


#endif